#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>
#include <omp.h>

// test read file

#define vi vector<int>

using namespace std;

int maxNode;
vi ans, connected;

void fast(){
    ios_base::sync_with_stdio(false); cin.tie(0); cout.tie(0);
}

vector<vector<int>> readInput(const string& filename, int& numNodes) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: could not open file " << filename << endl;
        exit(1);
    }

    file >> numNodes;
    int numEdges;
    file >> numEdges;

    vector<vector<int>> graph(numNodes, vector<int>(numNodes, 0));
    for (int i = 0; i < numEdges; ++i) {
        int u, v;
        file >> u >> v;
        graph[u][v] = 1;
        graph[v][u] = 1;
        graph[u][u] = 1;
        graph[v][v] = 1;
    }

    file.close();
    return graph;

}

int getLength(int numThreads){
    int length = 0;
    while(numThreads > 1){
        length++;
        numThreads /= 2;
    }
    return length;
}

int calculateSolution(int threadUsed){
    int solutionCount = 2;
    while(solutionCount <= threadUsed){
        solutionCount *= 2;
    }
    return solutionCount*2;
}

void getOption(int tid, int optionLength, vi &option){
    int it = 0;

    while(tid > 0){
        option[it] = tid %2 == 1;
        tid /= 2;
        it++;
    }
}

void solve(vi &selected, vi &visited, vector<vi> &matrix, int current, int node, int used, int maxEdge, int coveredNode){
    if(current == node || used >= maxNode || maxEdge < node - coveredNode) return;
    if(used < maxNode){
        //select current node
        selected[current] = 1;
        used+=1;

        int coveredNode = 0;
        for(int it1=0; it1<node; it1++){
            visited[it1] += matrix[current][it1];
            coveredNode += visited[it1] > 0;
        }

        if(coveredNode == node){
            if(used < maxNode){
                maxNode = used;
                #pragma omp critical
                for(int jt1=0; jt1<node; jt1++){
                    ans[jt1] = selected[jt1];
                }
            }
        } 
        solve(selected, visited, matrix, current+1, node, used, maxEdge-connected[current], coveredNode);
        
        selected[current] = 0;
        coveredNode = 0;
        used-=1;

        for(int it2=0; it2<node; it2++){
            visited[it2] -= matrix[current][it2];
            coveredNode += visited[it2] > 0;
        }
    }
    solve(selected, visited, matrix, current+1, node, used, maxEdge-connected[current], coveredNode);
}

vi preSolve(vi &option, vector<vi> &matrix, vi &selected, vi &visited, int node, int maxEdge){
    int used = 0, coveredNode = 0;
    for(int x=0; x<option.size(); x++){
        selected[x] = option[x];
        if(selected[x]){
            used++;
            coveredNode = 0;
            for(int j=0; j<node; j++){
                visited[j] += matrix[x][j];
                coveredNode += visited[j] > 0;
            }

            if(coveredNode == node){
                if(used < maxNode){
                    maxNode = used;
                    #pragma omp critical
                    for(int k=0; k<node; k++){
                        ans[k] = selected[k];
                    }
                }
            } 
        }
        maxEdge -= connected[x];
    }
    vi data{used, maxEdge, coveredNode};
    return data;
}

int main(int argc, char *argv[]){

    string inputFilename = argv[1];
    string outputFilename = argv[2];

    fast();
    
    int numNodes;
    vector<vi> matrix = readInput(inputFilename, numNodes);

    maxNode = numNodes;
    int maxEdge = 0;
    connected = vi(numNodes,0);
    for(int i = 0; i < numNodes; i++){
        for(int j = 0; j < numNodes; j++){
            connected[i] += matrix[i][j];
        }
        maxEdge += connected[i];
    }

    vi selected(numNodes,0), visited(numNodes,0), option, setting;

    ans = vi(numNodes,0);

    clock_t start, stop;
    start = clock();

    if(numNodes < 8 || omp_get_max_threads()<= 4){
        solve(selected, visited, matrix, 0, numNodes, 0, maxEdge, 0);
    }
    else{
        int threadUsed = min(12,omp_get_max_threads());
        int baseSolution = calculateSolution(threadUsed);
        int bitLength = getLength(baseSolution);
        #pragma omp parallel num_threads(threadUsed) shared(ans, maxNode) private(option, selected, visited, setting)
        {   
            #pragma omp for schedule(dynamic, 1)
            for(int iterator=0; iterator<baseSolution; iterator++){
                vi selected(numNodes,0), visited(numNodes,0), option(bitLength,0);
                getOption(iterator, bitLength, option);
                setting = preSolve(option, matrix, selected, visited, numNodes, maxEdge);
                solve(selected, visited, matrix, bitLength, numNodes, setting[0], setting[1], setting[2]);
            }
        }
    }

    stop = clock();
    double timeDifference = ((double)(stop - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %.3f seconds\n", timeDifference);
    printf("Minimum Node: %d\n", maxNode);
    for(int i=0;i<numNodes;i++){
        printf("%d", ans[i]);
    }

    ofstream file(outputFilename);
    for(int i=0;i<numNodes;i++){
        file << ans[i];
    }

}

// g++ -O2 -o solution solution.cpp
// ./solution ./testcase/grid-16-24 output
// ./solution ./testcase/ring-35-35 output

//grid-30-49 = 0.066
// Minimum Node: 8
// 001001010100000100101000000001

//ring-25-25 = 0.010
// Minimum Node: 9
// 0010011110100001100010000

//ring-35-35 = 2.502
// Minimum Node: 12
// 01100111000000110001000100100001010

//grid-40-67 = 23.019
// Minimum Node: 11
// 1100010001000110000000001001101000000001

//ring-40-40 = 77.727
// Minimum Node: 14
// 1001011010001110001011100100000000000100

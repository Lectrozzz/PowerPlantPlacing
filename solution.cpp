#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>

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

void solve(vi &selected, vi &visited, vector<vi> &matrix, int current, int node, int used, int maxEdge, int coveredNode){
    if(current == node || used >= maxNode || maxEdge < node - coveredNode) return;
    if(used < maxNode){
        //select current node
        selected[current] = 1;

        int coveredNode = 0;
        for(int i=0; i<node; i++){
            visited[i] += matrix[current][i];
            coveredNode += visited[i] > 0;
        }

        if(coveredNode == node){
            if(used < maxNode){
                maxNode = used;
                for(int i=0; i<node; i++){
                    ans[i] = selected[i];
                }
            }
        } 
        solve(selected, visited, matrix, current+1, node, used+1, maxEdge-connected[current], coveredNode);
        
        selected[current] = 0;
        coveredNode = 0;
        //todo: optimize this part
        for(int i=0; i<node; i++){
            visited[i] -= matrix[current][i];
            coveredNode += visited[i] > 0;
        }
    }
    solve(selected, visited, matrix, current+1, node, used, maxEdge-connected[current], coveredNode);
}

vi preSolve(vi &option, vector<vi> &matrix, vi &selected, vi &visited, int node, int maxEdge){
    int used = 0, coveredNode = 0;
    for(int i=0; i<option.size(); i++){
        selected[i] = option[i];
        used += option[i];
        if(selected[i]){
            for(int j=0; j<node; j++){
                visited[j] += matrix[i][j];
                coveredNode += visited[j] > 0;
            }

            if(coveredNode == node){
                if(used < maxNode){
                    maxNode = used;
                    for(int i=0; i<node; i++){
                        ans[i] = selected[i];
                    }
                }
            } 
        }
        maxEdge -= connected[i];
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

    vi selected(numNodes,0), visited(numNodes,0);

    ans = vi(numNodes,0);

    clock_t start, stop;
    start = clock();

    if(numNodes < 8){
        solve(selected, visited, matrix, 0, numNodes, 0, maxEdge, 0);
    }
    else{
        vi selected2(numNodes,0), selected3(numNodes,0), selected4(numNodes,0);
        vi visited2(numNodes,0), visited3(numNodes,0), visited4(numNodes,0);
        #pragma omp parallel sections num_threads(4) shared(ans)
        {   
            #pragma omp section
            {
                vi option4{1,1};
                vi setting4 = preSolve(option4, matrix, selected4, visited4, numNodes, maxEdge);
                solve(selected4, visited4, matrix, 1, numNodes, setting4[0], setting4[1], setting4[2]);
            }
            
            #pragma omp section
            {
                vi option3{1,0};
                vi setting3 = preSolve(option3, matrix, selected3, visited3, numNodes, maxEdge);
                solve(selected3, visited3, matrix, 1, numNodes, setting3[0], setting3[1], setting3[2]);
            }

            #pragma omp section
            {
                vi option2{0,1};
                vi setting2 = preSolve(option2, matrix, selected2, visited2, numNodes, maxEdge);
                solve(selected2, visited2, matrix, 1, numNodes, setting2[0], setting2[1], setting2[2]);
            }
            
            #pragma omp section
            {   
                vi option1{0,0};
                vi setting = preSolve(option1, matrix, selected, visited, numNodes, maxEdge);
                solve(selected, visited, matrix, 2, numNodes, setting[0], setting[1], setting[2]);
            }
        }
    }

    stop = clock();
    double timeDifference = ((double)(stop - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %.3f seconds\n", timeDifference);

    ofstream file(outputFilename);
    for(int i=0;i<numNodes;i++){
        file << ans[i];
    }

}

// g++ -O2 -o solution solution.cpp
// ./solution ./testcase/grid-16-24 output

//grid-30-49 = 0.052

//ring-25-25 = 0.026

//ring-35-35 = 4.988
// 00100111000000110001000001100001110

//grid-40-67 = 15.305

//ring-40-40 = 83.492
// 0011001010001110100011100100000100000010 = 14nodes
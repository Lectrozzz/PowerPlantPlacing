#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <math.h>

#define vi vector<int>
#define pii pair<int,int>

using namespace std;

int minNode, lowerBound=0;
vi ans;
vector<pii> priorityList;

bool piiGreater(const pii &a, const pii &b){
    if(a.first == b.first) return (a.second < b.second);
    return a.first > b.first;
}

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
    while(solutionCount < threadUsed){
        solutionCount *= 2;
    }
    return solutionCount*16;
}

void getOption(int tid, int optionLength, vi &option){
    int it = 0;

    while(tid > 0){
        option[it] = tid %2 == 1;
        tid /= 2;
        it++;
    }
}

int findMinimumNode(vector<vi> &matrix, vi &selected, vi &visited, int numNodes){
    int minimumNode = 0;

    for(int i=0; i<numNodes; i++){
        int index = priorityList[i].second, connectedTo = priorityList[i].first;
        bool shouldSelect = false;
        for(int j=0; j<numNodes; j++){
            if(matrix[index][j] == 1 && visited[j] == 0){
                shouldSelect = true;
            }
            visited[j] = matrix[index][j] | visited[j];
        }
        if(shouldSelect){
            selected[index] = 1;
            minimumNode+=1;
        }
    }

    for(int i=0; i<numNodes; i++){
        ans[i] = selected[i];
    }

    return minimumNode;
}

vi preSolve(vi &option, vector<vi> &matrix, vi &selected, vi &visited, int node, int maxEdge){
    int used = 0, coveredNode = 0, shouldSelect = 0;
    for(int x=0; x<option.size(); x++){
        int index = priorityList[x].second, connectedTo = priorityList[x].first; 
        selected[index] = option[x];
        if(option[x]){
            used++;
            coveredNode = 0;
            for(int j=0; j<node; j++){
                if(matrix[index][j] == 1 && visited[j] == 0){
                    shouldSelect = 1;
                }
                visited[j] += matrix[index][j];
                coveredNode += visited[j] > 0;
            }

            if(coveredNode == node){
                if(used < minNode){
                    minNode = used;
                    for(int k=0; k<node; k++){
                        ans[k] = selected[k];
                    }
                }
            } 
        }
        maxEdge -= connectedTo;
        if(!shouldSelect && option[x] || (used >= minNode)){
            vi data{used, maxEdge, coveredNode, 0};
            return data;
        }
    }
    vi data{used, maxEdge, coveredNode, shouldSelect};
    return data;
}

void solve(vi &selected, vi &visited, vector<vi> &matrix, int current, int node, int used, int maxEdge, int coveredNode){
    if(current == node || used >= minNode || maxEdge < node - coveredNode) return;
    int index = priorityList[current].second, connectedTo = priorityList[current].first;
    if(used < minNode){
        //select current node
        selected[index] = 1;
        used+=1;

        bool shouldSelect = false;

        int coveredNode = 0;
        for(int it1=0; it1<node; it1++){
            if(matrix[index][it1] == 1 && visited[it1] == 0){
                shouldSelect = true;
            }
            visited[it1] += matrix[index][it1];
            coveredNode += visited[it1] > 0;
        }

        if(coveredNode == node){
            if(used < minNode){
                minNode = used;
                for(int jt1=0; jt1<node; jt1++){
                    ans[jt1] = selected[jt1];
                }
            }
        }
        
        if(shouldSelect) solve(selected, visited, matrix, current+1, node, used, maxEdge-connectedTo, coveredNode);
        
        selected[index] = 0;
        used-=1;

        coveredNode = 0;
        for(int it2=0; it2<node; it2++){
            visited[it2] -= matrix[index][it2];
            coveredNode += visited[it2] > 0;
        }
    }
    solve(selected, visited, matrix, current+1, node, used, maxEdge-connectedTo, coveredNode);
}

int main(int argc, char *argv[]){

    string inputFilename = argv[1];
    string outputFilename = argv[2];

    fast();
    
    int numNodes;
    vector<vi> matrix = readInput(inputFilename, numNodes);

    vi selected(numNodes,0), visited(numNodes,0), option, setting;
    ans = vi(numNodes,0);   

    int maxEdge = 0;
    //add priority check
    for(int i = 0; i < numNodes; i++){
        int totalConnected = 0;
        for(int j = 0; j < numNodes; j++){
            totalConnected += matrix[i][j];
        }
        maxEdge += totalConnected;
        priorityList.push_back({totalConnected, i});
    }

    sort(priorityList.begin(), priorityList.end(), piiGreater);

    minNode = findMinimumNode(matrix, selected, visited, numNodes);

    if(numNodes < 8 || omp_get_max_threads()<= 2){
        vi selected(numNodes,0), visited(numNodes,0);
        solve(selected, visited, matrix, 0, numNodes, 0, maxEdge, 0);
    }
    else{
        int threadUsed = min(12,omp_get_max_threads());
        int baseSolution = pow(2, min(getLength(calculateSolution(threadUsed)), numNodes/2));
        int bitLength = getLength(baseSolution);
        #pragma omp parallel num_threads(threadUsed) shared(ans, minNode, matrix) private(option, selected, visited, setting)
        {   
            #pragma omp for schedule(dynamic, 1)
            for(int iterator=0; iterator<baseSolution; iterator++){
                vi selected(numNodes,0), visited(numNodes,0), option(bitLength,0);
                getOption(iterator, bitLength, option);
                setting = preSolve(option, matrix, selected, visited, numNodes, maxEdge);
                if(setting[3]) solve(selected, visited, matrix, bitLength, numNodes, setting[0], setting[1], setting[2]);
            }
        }
    }

    ofstream file(outputFilename);
    for(int i=0;i<numNodes;i++){
        file << ans[i];
    }
    return 0;
}

// g++ -O2 -fopenmp -o solution solution.cpp

//grid-30-49 = 0.024
// Minimum Node: 8
// 011100000100110100001000000000

//ring-25-25 = 0.008
// Minimum Node: 9
// 1100000001010100010100101

//ring-35-35 = 1.406
// Minimum Node: 12
// 11000000000111000111000110000100001

//grid-40-67 = 7.383
// Minimum Node: 11
// 1001001100100011000000100000000010100001

//ring-40-40 = 28.010
// Minimum Node: 14
// 1001000010000000001011001101001100100101

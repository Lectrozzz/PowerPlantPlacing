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

void solve(vi &selected, vi &visited, vector<vi> &matrix, int current, int node, int used, int nodeLeft, int total){
    if(current == node || used >= maxNode || nodeLeft < node - total ) return;
    if(used < maxNode){
        //select current node
        selected[current] = 1;

        int total = 0;
        for(int i=0; i<node; i++){
            visited[i] += matrix[current][i];
            total += visited[i] > 0;
        }

        if(total == node){
            if(used < maxNode){
                maxNode = used;
                for(int i=0; i<node; i++){
                    ans[i] = selected[i];
                }
            }
        } 
        solve(selected, visited, matrix, current+1, node, used+1, nodeLeft-connected[current], total);
        
        selected[current] = 0;
        for(int i=0; i<node; i++){
            visited[i] -= matrix[current][i];
            total -= visited[i] > 0;
        }
    }
    solve(selected, visited, matrix, current+1, node, used, nodeLeft-connected[current], total);
}

int main(int argc, char *argv[]){

    string inputFilename = argv[1];
    string outputFilename = argv[2];

    fast();
    
    int numNodes;
    vector<vi> matrix = readInput(inputFilename, numNodes);

    maxNode = numNodes;
    vi selected(numNodes,0), visited(numNodes,0);
    ans = vi(numNodes,0);

    int maximumNode = 0;
    connected = vi(numNodes,0);
    for(int i = 0; i < numNodes; i++){
        for(int j = 0; j < numNodes; j++){
            connected[i] += matrix[i][j];
        }
        maximumNode += connected[i];
    }

    clock_t start, stop;
    start = clock();

    solve(selected, visited, matrix, 0, numNodes, 0, maximumNode, 0);

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

//grid-30-49 = 0.061
// 101110000010000000100101000000

//ring-25-25 = 0.021
// 1100110000010100000101010

//ring-35-35 = 7.716
// 11000000000111000111000110000100001

//grid-40-67 = 24.091
// 1111001101000000010000100000000010000001

//ring-40-40 = 178.643
// 1101010000110001010101010000000010011000 = 14 nodes
// Лабораторная работа №3
// Semga67
// g++ -std=c++17 main.cpp -o semga.exe
//  ./semga.exe
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

using namespace std;

struct Counters {
    long long builds = 0;
    long long queries = 0;
    long long updates = 0;
};

// 1D RSQ prefix  
class PrefixSum1D {
private:
    vector<int> pref;
    Counters* counter;
    
public:
    PrefixSum1D(const vector<int>& arr, Counters* cnt) : counter(cnt) {
        int n = arr.size();
        pref.resize(n + 1, 0);
        
        for (int i = 0; i < n; i++) {
            pref[i + 1] = pref[i] + arr[i];
            counter->builds++;
        }
    }
    
    int query(int l, int r) {
        counter->queries += 2;
        return pref[r + 1] - pref[l];
    }
};

// 2D RSQ prefix 
class PrefixSum2D {
private:
    vector<vector<int>> pref;
    Counters* counter;
    
public:
    PrefixSum2D(const vector<vector<int>>& mat, Counters* cnt) : counter(cnt) {
        int n = mat.size();
        int m = mat[0].size();
        pref.resize(n + 1, vector<int>(m + 1, 0));
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                pref[i + 1][j + 1] = pref[i][j + 1] + pref[i + 1][j] - pref[i][j] + mat[i][j];
                counter->builds += 4;
            }
        }
    }
    
    int query(int x1, int y1, int x2, int y2) {
        counter->queries += 4;
        return pref[x2 + 1][y2 + 1] - pref[x1][y2 + 1] - pref[x2 + 1][y1] + pref[x1][y1];
    }
};

//RMQ 
class RMQPrecompute {
private:
    vector<vector<int>> st;
    Counters* counter;
    
public:
    RMQPrecompute(const vector<int>& arr, Counters* cnt) : counter(cnt) {
        int n = arr.size();
        st.resize(n, vector<int>(n));
        
        for (int i = 0; i < n; i++) {
            st[i][i] = arr[i];
            counter->builds++;
            
            for (int j = i + 1; j < n; j++) {
                if (arr[j] < st[i][j - 1]) {
                    st[i][j] = arr[j];
                } else {
                    st[i][j] = st[i][j - 1];
                }
                counter->builds++;
            }
        }
    }
    
    int query(int l, int r) {
        counter->queries++;
        return st[l][r];
    }
};

//Sqrt 
template<typename Op>
class SqrtDecomposition {
private:
    vector<int> data;
    vector<int> blocks;
    int blockSize;
    Op operation;
    int neutral;
    Counters* counter;
    
public:
    SqrtDecomposition(const vector<int>& arr, Op op, int neutralVal, Counters* cnt) 
        : operation(op), neutral(neutralVal), counter(cnt) {
        data = arr;
        int n = arr.size();
        blockSize = max(1, (int)sqrt(n));
        int numBlocks = (n + blockSize - 1) / blockSize;
        blocks.resize(numBlocks, neutral);
        
        for (int i = 0; i < n; i++) {
            int b = i / blockSize;
            blocks[b] = operation(blocks[b], arr[i]);
            counter->builds++;
        }
    }
    
    int query(int l, int r) {
        int res = neutral;
        int lb = l / blockSize;
        int rb = r / blockSize;
        
        if (lb == rb) {
            for (int i = l; i <= r; i++) {
                res = operation(res, data[i]);
                counter->queries++;
            }
        } else {
            for (int i = l; i < (lb + 1) * blockSize; i++) {
                res = operation(res, data[i]);
                counter->queries++;
            }
            for (int b = lb + 1; b < rb; b++) {
                res = operation(res, blocks[b]);
                counter->queries++;
            }
            for (int i = rb * blockSize; i <= r; i++) {
                res = operation(res, data[i]);
                counter->queries++;
            }
        }
        return res;
    }
    
    void update(int idx, int val) {
        int b = idx / blockSize;
        data[idx] = val;
        
        blocks[b] = neutral;
        int start = b * blockSize;
        int end = min(start + blockSize, (int)data.size());
        for (int i = start; i < end; i++) {
            blocks[b] = operation(blocks[b], data[i]);
            counter->updates++;
        }
    }
};

// Segment Tree
template<typename T>
class SegmentTree {
private:
    vector<T> tree;
    int n;
    T (*merge)(T, T);
    T neutral;
    Counters* counter;
    
    void build(const vector<T>& arr, int node, int l, int r) {
        if (l == r) {
            tree[node] = arr[l];
            counter->builds++;
            return;
        }
        int mid = (l + r) / 2;
        build(arr, node * 2, l, mid);
        build(arr, node * 2 + 1, mid + 1, r);
        tree[node] = merge(tree[node * 2], tree[node * 2 + 1]);
        counter->builds++;
    }
    
    T query(int node, int l, int r, int ql, int qr) {
        if (qr < l || ql > r) return neutral;
        if (ql <= l && r <= qr) {
            counter->queries++;
            return tree[node];
        }
        int mid = (l + r) / 2;
        T leftRes = query(node * 2, l, mid, ql, qr);
        T rightRes = query(node * 2 + 1, mid + 1, r, ql, qr);
        counter->queries++;
        return merge(leftRes, rightRes);
    }
    
    void update(int node, int l, int r, int idx, T val) {
        if (l == r) {
            tree[node] = val;
            counter->updates++;
            return;
        }
        int mid = (l + r) / 2;
        if (idx <= mid) {
            update(node * 2, l, mid, idx, val);
        } else {
            update(node * 2 + 1, mid + 1, r, idx, val);
        }
        tree[node] = merge(tree[node * 2], tree[node * 2 + 1]);
        counter->updates++;
    }
    
public:
    SegmentTree(const vector<T>& arr, T (*mergeFunc)(T, T), T neutralVal, Counters* cnt)
        : merge(mergeFunc), neutral(neutralVal), counter(cnt) {
        n = arr.size();
        tree.resize(4 * n);
        build(arr, 1, 0, n - 1);
    }
    
    T query(int l, int r) {
        return query(1, 0, n - 1, l, r);
    }
    
    void update(int idx, T val) {
        update(1, 0, n - 1, idx, val);
    }
};

// Fenwick Tree 
class FenwickTree {
private:
    vector<int> bit;
    Counters* counter;
    
public:
    FenwickTree(const vector<int>& arr, Counters* cnt) : counter(cnt) {
        int n = arr.size();
        bit.resize(n + 1, 0);
        
        for (int i = 0; i < n; i++) {
            update(i, arr[i]);
        }
    }
    
    void update(int idx, int delta) {
        idx++;
        int n = bit.size();
        while (idx < n) {
            bit[idx] += delta;
            counter->updates++;
            idx += idx & -idx;
        }
    }
    
    int sum(int idx) {
        idx++;
        int res = 0;
        while (idx > 0) {
            res += bit[idx];
            counter->queries++;
            idx -= idx & -idx;
        }
        return res;
    }
    
    int query(int l, int r) {
        counter->queries++;
        return sum(r) - sum(l - 1);
    }
};

// Sparse Table 
class SparseTable {
private:
    vector<vector<int>> st;
    vector<int> log;
    Counters* counter;
    
public:
    SparseTable(const vector<int>& arr, Counters* cnt) : counter(cnt) {
        int n = arr.size();
        log.resize(n + 1);
        log[1] = 0;
        for (int i = 2; i <= n; i++) {
            log[i] = log[i / 2] + 1;
        }
        
        int K = log[n] + 1;
        st.resize(n, vector<int>(K));
        
        for (int i = 0; i < n; i++) {
            st[i][0] = arr[i];
            counter->builds++;
        }
        
        for (int j = 1; j < K; j++) {
            for (int i = 0; i + (1 << j) <= n; i++) {
                st[i][j] = min(st[i][j - 1], st[i + (1 << (j - 1))][j - 1]);
                counter->builds++;
            }
        }
    }
    
    int query(int l, int r) {
        int j = log[r - l + 1];
        counter->queries++;
        return min(st[l][j], st[r - (1 << j) + 1][j]);
    }
};

int sumMerge(int a, int b) { return a + b; }
int minMerge(int a, int b) { return min(a, b); }

void printResults(const string& name, const Counters& cnt, int qCount) {
    cout << name << ":\n";
    cout << "  Build: " << cnt.builds << " operations\n";
    cout << "  Query (avg): " << (double)cnt.queries / qCount << "\n";
    if (cnt.updates > 0) {
        cout << "  Update (avg): " << (double)cnt.updates / qCount << "\n";
    }
    cout << "\n";
}

int main() {
    const int n = 20000;
    const int queries = 1000;
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 1000);
    
    vector<int> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = dis(gen);
    }
    
    vector<vector<int>> mat(100, vector<int>(100));
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            mat[i][j] = dis(gen);
        }
    }
    
    uniform_int_distribution<> posDis(0, n - 1);
    uniform_int_distribution<> posDis2D(0, 99);
    
    Counters cnt1;
    PrefixSum1D ps1(arr, &cnt1);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen);
        int r = posDis(gen);
        if (l > r) swap(l, r);
        ps1.query(l, r);
    }
    printResults("1D RSQ via prefix sums", cnt1, queries);
    
    Counters cnt2;
    PrefixSum2D ps2(mat, &cnt2);
    for (int q = 0; q < queries; q++) {
        int x1 = posDis2D(gen), y1 = posDis2D(gen);
        int x2 = posDis2D(gen), y2 = posDis2D(gen);
        if (x1 > x2) swap(x1, x2);
        if (y1 > y2) swap(y1, y2);
        ps2.query(x1, y1, x2, y2);
    }
    printResults("2D RSQ via prefix sums", cnt2, queries);
    
    Counters cnt3;
    RMQPrecompute rmqPre(arr, &cnt3);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen);
        int r = posDis(gen);
        if (l > r) swap(l, r);
        rmqPre.query(l, r);
    }
    printResults("RMQ via precomputation", cnt3, queries);
    
    Counters cnt4;
    SqrtDecomposition<int(*)(int,int)> sqrtSum(arr, sumMerge, 0, &cnt4);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        sqrtSum.query(l, r);
    }
    printResults("Sqrt Decomposition RSQ", cnt4, queries);
    
    Counters cnt5;
    SqrtDecomposition<int(*)(int,int)> sqrtMin(arr, minMerge, INT_MAX, &cnt5);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        sqrtMin.query(l, r);
    }
    printResults("Sqrt Decomposition RMQ", cnt5, queries);
    
    Counters cnt6;
    SegmentTree<int> segSum(arr, sumMerge, 0, &cnt6);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        segSum.query(l, r);
    }
    printResults("Segment Tree RSQ", cnt6, queries);
    
    Counters cnt7;
    SegmentTree<int> segMin(arr, minMerge, INT_MAX, &cnt7);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        segMin.query(l, r);
    }
    printResults("Segment Tree RMQ", cnt7, queries);
    
    Counters cnt8;
    FenwickTree fenw(arr, &cnt8);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        fenw.query(l, r);
    }
    printResults("Fenwick Tree RSQ", cnt8, queries);
    
    Counters cnt9;
    SparseTable spTable(arr, &cnt9);
    for (int q = 0; q < queries; q++) {
        int l = posDis(gen), r = posDis(gen);
        if (l > r) swap(l, r);
        spTable.query(l, r);
    }
    printResults("Sparse Table RMQ", cnt9, queries);
    
    return 0;
}
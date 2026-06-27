#include <bits/stdc++.h>
using namespace std;

#define MAXN 10000

int dp[MAXN][MAXN];
int p[MAXN];
int v[MAXN];
int N, C;

int solve(){
  for(int c = 1; c <= C; c++){
    for(int n = 1; n <= N; n++){
      if(p[n] <= c) dp[c][n] = max(dp[c][n-1], 
                                   dp[c - p[n]][n-1] + v[n]);
      else dp[c][n] = dp[c][n-1];
    }
  }
  return dp[C][N];
}

int main (int argc, char *argv[]) {
  cin >> N >> C;

  for(int i = 1; i <= N; i++) cin >> p[i] >> v[i];

  cout << solve() << '\n';
  return 0;
}















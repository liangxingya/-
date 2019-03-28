#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

const int piece = 800;
const int source = 100;
const int max_thread_num = 20;
const int x_start = piece * 0.1, y_start = piece * 0.1;

int n_ter = 500;
double matrix_pre[piece + 10][piece + 10];
double matrix_next[piece + 10][piece + 10];
bool run_break;
int n_pthread;
pthread_t tid[max_thread_num];
pthread_cond_t c;
pthread_mutex_t m;
bool finish[max_thread_num];
int area;
bool flag[max_thread_num];
vector<bool> v;

void init()
{
	run_break = false;
	matrix_pre[x_start][y_start] = source;
	memset(matrix_pre, 0, sizeof(matrix_pre));
	memset(matrix_next, 0, sizeof(matrix_next));
	pthread_cond_init(&c, NULL);
	pthread_mutex_init(&m, NULL);
	for(int i = 0; i < n_pthread; i++) {
		v.push_back(false);
	}
}

bool judge(int x, int y)
{

	if(x == 0 || y == 0 || x >= piece - 1 || y >= piece - 1 || ((x * x + y * y) >= piece * piece))
		return false;
	return true;
}


bool is_ok(int left, int right)
{
	for(int i = 0; i < piece; i++) {
		for(int j = left; j < right; j++) {
			if(fabs(matrix_pre[i][j] - matrix_next[i][j]) > 0.001)
				return false;
		}
	}
	return true;
}

void print()
{
	for(int i = 0; i < piece; i++) {
		for(int j = 0; j < piece; j++) {
			cout << matrix_pre[i][j] << " ";
		}
		cout << endl;
	}
	cout << "---------------" << endl;
	for(int i = 0; i < piece; i++) {
		for(int j = 0; j < piece; j++) {
			cout << matrix_next[i][j] << " ";
		}
		cout << endl;
	}
	
}

bool check_finish(int id) {
	for(int i = 0; i < n_pthread; i++) {
		if(finish[i] == false)
			return false;
	}
	return true;
}

bool check_v() {
	for(int i = 0; i < n_pthread; i++) {
		if(v[i] == false)
			return false;
	}
	return true;
}

void *run(void *argp)
{
	int id = *((int *)argp);
	free(argp);
	int left = id * area;
	int right = (id + 1) * area;
	int cnt = 1;
	while(true)
	{
		v[id] = false;
		finish[id] = false;
		flag[id] = false;
		for(int i = 0; i < piece; i++) 
		{
			for(int j = left; j < right; j++)
			{
				if(judge(i, j))
				{
					matrix_next[i][j] = (matrix_pre[i + 1][j] + matrix_pre[i - 1][j] + matrix_pre[i][j - 1] + matrix_pre[i][j + 1]) / 4;
					//cout << matrix_next[i][j] << endl;
				}
			}
		}

		if(x_start < right && x_start >= left)
			matrix_next[x_start][y_start] = source;
		if(is_ok(left, right))
			v[id] = true;
		pthread_mutex_lock(&m);
		finish[id] = true;
		if(check_finish(id)) {
			if(check_v())
				run_break = true;
			for(int i = 0; i < n_pthread; i++) {
				if(flag[i]) {
					pthread_cond_broadcast(&c);
					break;
				}
			}
		}
		pthread_mutex_unlock(&m);

		pthread_mutex_lock(&m);
		while(check_finish(id) == false) {
			flag[id] = true;
			pthread_cond_wait(&c, &m);
		}
		flag[id] = false;
		pthread_mutex_unlock(&m);

		if(run_break)
			break;
		for(int i = 0; i < piece; i++)
			for(int j = left; j < right; j++)
				matrix_pre[i][j] = matrix_next[i][j];
		cnt += 1;
		if(cnt % 100 == 0)
			cout << "the pthread " << id << " has run " << cnt << endl;
	} 
}

int main(int argc, char **argv)
{
	if(argc != 2) {
		cout << "we need tow argcs" << endl;
		return 0;
	}
	n_pthread = atoi(argv[1]);
	area = piece / n_pthread;
	init();
	
	//print();
	int *ptr;
    for(int i = 0; i < n_pthread; i++) {
    	ptr = (int *)malloc(sizeof(int));
    	*ptr = i;
    	pthread_create(&tid[i], NULL, run, (void *)ptr);
    }

    for(int i = 0; i < n_pthread; i++) {
    	pthread_join(tid[i], NULL);
    }
	
	int ans = 0;
	for(int i = 0; i < piece; i++) {
		for(int j = 0; j < piece; j++) {
			if(matrix_next[i][j] > 50)
				ans++;
		}
	}

	cout << ans << endl;
	cout << ans * 1.0  / (piece * piece) << endl;

	return 0;
}

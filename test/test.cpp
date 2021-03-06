#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iomanip>
#include <chrono>
#include <omp.h>
using namespace std;

double dsecnd()
{
    return chrono::duration<double, ratio<1, 1>>
        (chrono::steady_clock::now() - chrono::steady_clock::time_point()).count();
}

double drand()
{
    return rand() * 1. / RAND_MAX;
}

extern "C"
void mul0(int n, double* a, double* b, double* c)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            double v = 0;
            for (int k = 0; k < n; k++)
                v += a[i*n + k]*b[k*n + j];

            c[i*n + j] = v;
        }
}

extern "C"
void mul1(int n, double* a, double* b, double* c)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            double v = 0;
            #pragma omp parallel for reduction(+:v)
            for (int k = 0; k < n; k++)
                v += a[i*n + k]*b[k*n + j];

            c[i*n + j] = v;
        }
}

extern "C"
void mul2(int n, double* a, double* b, double* c)
{
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
        {
            double v = a[i*n + k];
            for (int j = 0; j < n; j++)
                c[i*n + j] += v*b[k*n + j];
        }
}

extern "C"
void mul3(int n, double* a, double* b, double* c)
{
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
        {
            double v = a[i*n + k];
            #pragma omp parallel for
            for (int j = 0; j < n; j++)
                c[i*n + j] += v*b[k*n + j];
        }
}

typedef void (*mul)(int, double*, double*, double*);

template <class Func>
void test(Func func, int n, double a[], double b[], double c[])
{
    double t0 = dsecnd();
    func(n, a, b, c);
    cout << "Matrix multiplication time: " << dsecnd() - t0 << endl;

    double err = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            double v = 0;
            for (int k = 0; k < n; k++)
                v += a[i*n + k]*b[k*n + j];
            err = max(err, abs(v - c[i*n + j]));
        }

    cout << "Matrix multiplication error: " << err << endl;
    cout << endl;
}

int main(int argc, char** argv)
{
    const int n = stoi(argv[1]);
    const int mix = stoi(argv[2]);
    mul mfuncs[4] = { &mul0, &mul1, &mul2, &mul3};
    const int N = 16;
    assert(n == N);
    double a[N*N];
    double b[N*N];
    double c[N*N] = { 0 };
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            a[i*n + j] = drand();
            b[i*n + j] = drand();
        }
    }
    test(*mfuncs[mix], n, a, b, c);
    return 0;
}

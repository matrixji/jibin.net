#include <vector>
#include <cblas.h>


int main() {
    int lda = 8;
    int ldb = 7;
    int ldc = 6;
    int M = 2;
    int N = 3;
    int K = 4;
    float alpha = 1.0;
    float beta = 0.0;

    std::vector<float> A {
        0.0, 0.7, 0.7, 0.1, 0.1, 0.0, 0.0, 0.0,
        0.0, 0.1, 0.9, 0.3, 0.3, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };

    std::vector<float> B {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.8, 0.1, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.4, 0.3, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.4, 0.3, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.2, 0.9, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };

    std::vector<float> C {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                M, N, K,
                alpha, &A[1], lda,
                &B[8], ldb, 
                beta, &C[1], ldc);
    
    for (auto i=0; i<C.size(); ++i) {
        if (i % ldc == 0) {
            printf("\n");
        }
        printf("%.2f ", C[i]);
    }
    printf("\n");

    return 0;
    
}

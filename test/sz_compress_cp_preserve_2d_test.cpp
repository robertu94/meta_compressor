#include "sz_compress_cp_preserve_2d.hpp"
#include "sz_decompress_cp_preserve_2d.hpp"
#include "sz_lossless.hpp"
#include "utils.hpp"
using namespace std;

template<typename T>
void 
transpose_2d(T * data, size_t r1, size_t r2){
    for(int i=0; i<r1; i++){
        for(int j=0; j<i; j++){
            T tmp = data[i*r2 + j];
            data[i*r1 + j] = data[j*r1 + i];
            data[j*r1 + i] = tmp;
        }
    }
}

int main(int argc, char ** argv){
    size_t num_elements = 0;
    double * U = readfile<double>(argv[1], num_elements);
    double * V = readfile<double>(argv[2], num_elements);
    int r1 = atoi(argv[3]);
    int r2 = atoi(argv[4]);
    double max_eb = atof(argv[5]);
    cout << U[r2 + 3] << " " << U[3*r2 + 1] << endl;
    transpose_2d(U, r1, r2);
    cout << U[r2 + 3] << " " << U[3*r2 + 1] << endl;
    transpose_2d(V, r1, r2);

    size_t result_size = 0;
    struct timespec start, end;
    int err = 0;
    err = clock_gettime(CLOCK_REALTIME, &start);
    cout << "start Compression\n";
    // unsigned char * result =  sz_compress_cp_preserve_2d_offline(U, V, r1, r2, result_size);
    // unsigned char * result =  sz_compress_cp_preserve_2d_offline_log(U, V, r1, r2, result_size);
    // unsigned char * result =  sz_compress_cp_preserve_2d_online(U, V, r1, r2, result_size, false, max_eb);
    unsigned char * result =  sz_compress_cp_preserve_2d_online_log(U, V, r1, r2, result_size, false, max_eb);
    unsigned char * result_after_lossless = NULL;
    size_t lossless_outsize = sz_lossless_compress(ZSTD_COMPRESSOR, 3, result, result_size, &result_after_lossless);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Compression time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    cout << "Compressed size = " << lossless_outsize << ", ratio = " << (num_elements*sizeof(double)) * 1.0/lossless_outsize << endl;
    free(result);
    // exit(0);
    err = clock_gettime(CLOCK_REALTIME, &start);
    size_t lossless_output = sz_lossless_decompress(ZSTD_COMPRESSOR, result_after_lossless, lossless_outsize, &result, result_size);
    double * dec_U = NULL;
    double * dec_V = NULL;
    // sz_decompress_cp_preserve_2d_offline<double>(result, r1, r2, dec_U, dec_V);
    // sz_decompress_cp_preserve_2d_offline_log<double>(result, r1, r2, dec_U, dec_V);
    // sz_decompress_cp_preserve_2d_online<double>(result, r1, r2, dec_U, dec_V);
    sz_decompress_cp_preserve_2d_online_log<double>(result, r1, r2, dec_U, dec_V);
    err = clock_gettime(CLOCK_REALTIME, &end);
    cout << "Decompression time: " << (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)1000000000 << "s" << endl;
    free(result_after_lossless);

    verify(U, dec_U, num_elements);
    verify(V, dec_V, num_elements);

    transpose_2d(dec_U, r1, r2);
    transpose_2d(dec_V, r1, r2);
    writefile((string(argv[1]) + ".out").c_str(), dec_U, num_elements);
    writefile((string(argv[2]) + ".out").c_str(), dec_V, num_elements);
    free(result);
    free(U);
    free(V);
    free(dec_U);
    free(dec_V);
}
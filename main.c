#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int **a;
    int **b;
    int **c;
    int row_size_a;
    int column_size_a;
    int row_size_b;
    int column_size_b;
    int rowNumber;
    int columnNumber;
    long double nanoTime;
    long double microTime;
    long double milliTime;
    long double secondTime;
}Data;

FILE *data_a;
FILE *data_b;
FILE *output_c_normal;
FILE *output_c_row;
FILE *output_c_element;

void readFiles(int argc, const char *argv[], Data *data);
void startMultiplication(Data *data);
void *multiplyPerMatrix(void *args);
void *multiplyPerRow(void *args);
void *multiplyPerElement(void *args);
void *rowMultiplication(void *args);
void *elementMultiplication(void *args);
void writeToFile(Data *data, int type);
void cloneStruct(Data *source, Data *destination);
void freeData(Data *data, int index);

int main(int argc, const char *argv[]) {
    Data *data = (Data *)malloc(3 * sizeof(Data));
    readFiles(argc, argv, data);
    startMultiplication(data);
    writeToFile(data, 0);
    writeToFile(data, 1);
    writeToFile(data, 2);
    for (int i = 0; i < 3; i++) {
        freeData(&data[i], i);
    }
    free(data);
    return 0;
}

void readFiles(int argc, const char *argv[], Data *data){
    // Open the files
    if(argc < 4){
        data_a = fopen("a.txt", "r");
        data_b = fopen("b.txt", "r");
        output_c_normal = fopen("c_per_matrix.txt", "w");
        output_c_row = fopen("c_per_row.txt", "w");
        output_c_element = fopen("c_per_element.txt", "w");
    }
    else{
        data_a = fopen(strcat(argv[1],".txt"), "r");
        data_b = fopen(strcat(argv[2],".txt"), "r");
        output_c_normal = fopen(strcat(argv[3],"_per_matrix.txt"), "w");
        output_c_row = fopen(strcat(argv[3],"_per_row.txt"), "w");
        output_c_element = fopen(strcat(argv[3],"_per_element.txt"), "w");
    }

    // Read the first matrix
    fscanf(data_a, "row=%d col=%d", &data[0].row_size_a, &data[0].column_size_a);
    data[0].a = (int **)malloc(data[0].row_size_a * sizeof(int *));
    data[1].row_size_a = data[0].row_size_a;
    data[2].row_size_a = data[0].row_size_a;
    data[1].column_size_a = data[0].column_size_a;
    data[2].column_size_a = data[0].column_size_a;
    for(int i = 0; i < data[0].row_size_a; i++){
        data[0].a[i] = (int *)malloc(data[0].column_size_a * sizeof(int));
    }
    for(int i = 0; i < data[0].row_size_a; i++){
        for(int j = 0; j < data[0].column_size_a; j++){
            fscanf(data_a, "%d", &data[0].a[i][j]);
        }
    }
    data[1].a = data[0].a;
    data[2].a = data[0].a;
    fclose(data_a);

    // Read the second matrix
    fscanf(data_b, "row=%d col=%d", &data[0].row_size_b, &data[0].column_size_b);
    data[0].b = (int **)malloc(data[0].row_size_b * sizeof(int *));
    data[1].row_size_b = data[0].row_size_b;
    data[2].row_size_b = data[0].row_size_b;
    data[1].column_size_b = data[0].column_size_b;
    data[2].column_size_b = data[0].column_size_b;
    for(int i = 0; i < data[0].row_size_b; i++){
        data[0].b[i] = (int *)malloc(data[0].column_size_b * sizeof(int));
    }
    for(int i = 0; i < data[0].row_size_b; i++){
        for(int j = 0; j < data[0].column_size_b; j++){
            fscanf(data_b, "%d", &data[0].b[i][j]);
        }
    }
    data[1].b = data[0].b;
    data[2].b = data[0].b;
    fclose(data_b);

    // Check if the matrices can be multiplied
    if(data[0].column_size_a != data[0].row_size_b){
        fprintf(output_c_element, "Matrix A's column size is not equal to Matrix B's row size\n");
        fprintf(output_c_row, "Matrix A's column size is not equal to Matrix B's row size\n");
        fprintf(output_c_normal, "Matrix A's column size is not equal to Matrix B's row size\n");
        fclose(output_c_element);
        fclose(output_c_row);
        fclose(output_c_normal);
        exit(0);
    }

    // Allocate memory for the result matrix
    data[0].c = (int **)malloc(data[0].row_size_a * sizeof(int *));
    data[1].c = (int **)malloc(data[1].row_size_a * sizeof(int *));
    data[2].c = (int **)malloc(data[2].row_size_a * sizeof(int *));
    for(int i = 0; i < data[0].row_size_a; i++){
        data[0].c[i] = (int *)malloc(data[0].column_size_b * sizeof(int));
        data[1].c[i] = (int *)malloc(data[1].column_size_b * sizeof(int));
        data[2].c[i] = (int *)malloc(data[2].column_size_b * sizeof(int));
    }
}
void startMultiplication(Data *data){
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    if(pthread_create(&thread1, NULL, multiplyPerMatrix, (void *)data)){
        fprintf(output_c_normal, "Error creating thread 1\n");
        fclose(output_c_normal);
        exit(0);
    }
    if(pthread_create(&thread2, NULL, multiplyPerRow, (void *)data)){
        fprintf(output_c_row, "Error creating thread 2\n");
        fclose(output_c_row);
        exit(0);
    }
    if(pthread_create(&thread3, NULL, multiplyPerElement, (void *)data)){
        fprintf(output_c_element, "Error creating thread 3\n");
        fclose(output_c_element);
        exit(0);
    }
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
}
void *multiplyPerMatrix(void *args){
    Data *data = (Data *)args;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i =0; i< data[0].row_size_a; i++){
        for(int j = 0; j < data[0].column_size_b; j++){
            data[0].c[i][j] = 0;
            for(int k = 0; k < data[0].column_size_a; k++){
                data[0].c[i][j] += data[0].a[i][k] * data[0].b[k][j];
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long totalNanoTime = (end.tv_sec - start.tv_sec) * 1000000000LL + end.tv_nsec - start.tv_nsec;
    data[0].nanoTime = totalNanoTime; // in nanoseconds
    data[0].microTime = totalNanoTime / 1000.0; // convert to microseconds
    data[0].milliTime = totalNanoTime / 1000000.0; // convert to milliseconds
    data[0].secondTime = totalNanoTime / 1000000000.0; // convert to seconds
    return NULL;
}
void *multiplyPerRow(void *args){
    Data *data = (Data *)args;
    pthread_t threads[data[1].row_size_a];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i = 0; i < data[1].row_size_a; i++){
        Data *currentData = (Data *)malloc(sizeof(Data));
        cloneStruct(&data[1], currentData);
        currentData->rowNumber = i;
        if(pthread_create(&threads[i], NULL, rowMultiplication, (void *)currentData)){
            fprintf(output_c_row, "Error creating thread\n");
            fclose(output_c_row);
            exit(0);
        }
    }
    for(int i = 0; i < data[1].row_size_a; i++){
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long totalNanoTime = (end.tv_sec - start.tv_sec) * 1000000000LL + end.tv_nsec - start.tv_nsec;
    data[1].nanoTime = totalNanoTime; // in nanoseconds
    data[1].microTime = totalNanoTime / 1000.0; // convert to microseconds
    data[1].milliTime = totalNanoTime / 1000000.0; // convert to milliseconds
    data[1].secondTime = totalNanoTime / 1000000000.0; // convert to seconds
    return NULL;
}
void *multiplyPerElement(void *args){
    Data *data = (Data *)args;
    pthread_t threads[data[2].row_size_a][data[2].column_size_b];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i=0; i< data[2].row_size_a;i++){
        for(int j=0; j< data[2].column_size_b;j++){
            Data *currentData = (Data *)malloc(sizeof(Data));
            cloneStruct(&data[2], currentData);
            data[2].c[i][j] = 0;
            currentData->rowNumber = i;
            currentData->columnNumber = j;
            if(pthread_create(&threads[i][j], NULL, elementMultiplication, (void *)currentData)){
                fprintf(output_c_element, "Error creating thread\n");
                fclose(output_c_element);
                exit(0);
            }
        }
    }
    for(int i=0; i< data[2].row_size_a;i++){
        for(int j=0; j< data[2].column_size_b;j++){
            pthread_join(threads[i][j], NULL);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long totalNanoTime = (end.tv_sec - start.tv_sec) * 1000000000LL + end.tv_nsec - start.tv_nsec;
    data[2].nanoTime = totalNanoTime; // in nanoseconds
    data[2].microTime = totalNanoTime / 1000.0; // convert to microseconds
    data[2].milliTime = totalNanoTime / 1000000.0; // convert to milliseconds
    data[2].secondTime = totalNanoTime / 1000000000.0; // convert to seconds
    return NULL;
}
void *rowMultiplication(void *args){
    Data *data = (Data *)args;
    for(int j = 0; j < data->column_size_b; j++){
        data->c[data->rowNumber][j] = 0;
        for(int k = 0; k < data->column_size_a; k++){
            data->c[data->rowNumber][j] += data->a[data->rowNumber][k] * data->b[k][j];
        }
    }
    free(data);
    return NULL;
}
void *elementMultiplication(void *args){
    Data *data = (Data *)args;
    for(int i=0; i<data->column_size_a;i++){
        data->c[data->rowNumber][data->columnNumber] += data->a[data->rowNumber][i] * data->b[i][data->columnNumber];
    }
    free(data);
    return NULL;
}
void writeToFile(Data *data, int type){
    if(type == 0){
        for(int i = 0; i < data[0].row_size_a; i++){
            for(int j = 0; j < data[0].column_size_b; j++){
                fprintf(output_c_normal, "%d ", data[0].c[i][j]);
            }
            fprintf(output_c_normal, "\n");
        }
        fprintf(output_c_normal, "\nTime: %LF nano seconds\n", data[0].nanoTime);
        fprintf(output_c_normal, "Time: %LF micro seconds\n", data[0].microTime);
        fprintf(output_c_normal, "Time: %LF milli seconds\n", data[0].milliTime);
        fprintf(output_c_normal, "Time: %LF seconds\n", data[0].secondTime);
        fprintf(output_c_normal, "\n Number of threads: 1\n");
        fclose(output_c_normal);
    }
    else if(type == 1){
        for(int i = 0; i < data[1].row_size_a; i++){
            for(int j = 0; j < data[1].column_size_b; j++){
                fprintf(output_c_row, "%d ", data[1].c[i][j]);
            }
            fprintf(output_c_row, "\n");
        }
        fprintf(output_c_row, "\nTime: %Lf nano seconds\n", data[1].nanoTime);
        fprintf(output_c_row, "Time: %Lf micro seconds\n", data[1].microTime);
        fprintf(output_c_row, "Time: %Lf milli seconds\n", data[1].milliTime);
        fprintf(output_c_row, "Time: %Lf seconds\n", data[1].secondTime);
        fprintf(output_c_row, "\n Number of threads: %d\n", data[1].row_size_a);
        fclose(output_c_row);
    }
    else if(type == 2){
        for(int i = 0; i < data[2].row_size_a; i++){
            for(int j = 0; j < data[2].column_size_b; j++){
                fprintf(output_c_element, "%d ", data[2].c[i][j]);
            }
            fprintf(output_c_element, "\n");
        }
        fprintf(output_c_element, "\nTime: %Lf nano seconds\n", data[2].nanoTime);
        fprintf(output_c_element, "Time: %Lf micro seconds\n", data[2].microTime);
        fprintf(output_c_element, "Time: %Lf milli seconds\n", data[2].milliTime);
        fprintf(output_c_element, "Time: %Lf seconds\n", data[2].secondTime);
        fprintf(output_c_element, "\n Number of threads: %d\n", data[2].row_size_a * data[2].column_size_b);
        fclose(output_c_element);
    }
}
void cloneStruct(Data *source, Data *destination){
    destination->a = source->a;
    destination->b = source->b;
    destination->c = source->c;
    destination->row_size_a = source->row_size_a;
    destination->column_size_a = source->column_size_a;
    destination->row_size_b = source->row_size_b;
    destination->column_size_b = source->column_size_b;
}
void freeData(Data *data, int index) {
    if (data != NULL) {
        if (data->a != NULL && index == 0) {
            for (int i = 0; i < data->row_size_a; i++) {
                free(data->a[i]);
            }
            free(data->a);
        }
        if (data->b != NULL && index == 0) {
            for (int i = 0; i < data->row_size_b; i++) {
                free(data->b[i]);
            }
            free(data->b);
        }
        if (data->c != NULL) {
            for (int i = 0; i < data->rowNumber; i++) {
                free(data->c[i]);
            }
            free(data->c);
        }
    }
}
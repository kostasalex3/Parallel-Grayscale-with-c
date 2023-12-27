#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bmp.h"
#include <time.h>
#include <mpi.h>


double **get_varh_apo_bmp(char *filename,int dimension);
int euresh_diastashs_txt_varwn(char *filename);
void free_2d_malloced_array(double **array,int dimension);
void display_2d_array(int **array,int dimension);
BMPImage_t* grayscale(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH,double **kernel,double coeff,int dimension_varwn);
BMPImage_t* horizontal_flip_bmp(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH);
BMPImage_t* vertical_flip_bmp(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH);
BMPImage_t* array_To_bmp(int Height,int Width,int array_local_me_pixels_R[Height][Width],int array_local_me_pixels_G[Height][Width],int array_local_me_pixels_B[Height][Width],BMPImage_t* bmpp);

struct timespec start, end;


int main(int argc, char *argv[])
{
        int rank,size;

        if (argc < 2) {
          printf("missing argument\n");
          exit(-1);
        }
    
    double coeff = 1.0f/9;
    int i,dimension;
    
    char kernel_directory[] = "weights.txt";

    char eikona[]="jounior.bmp";

    char *metatropi_char = argv[1];
    int metatropi = atoi(metatropi_char);
    
    if(metatropi >= 3){
        printf("Choices are: N=0 No conversion\nN=1 vertical\nN=2 Horizontal \n");
    }
    
    dimension = euresh_diastashs_txt_varwn(kernel_directory);
        //dimiourgia 2d metavlitou array
       
        double **kernel ;
        
        kernel = (double**)malloc(dimension*sizeof(double*));
        for(i=0;i<dimension;i++){
                kernel[i] =  (double*)malloc(dimension*sizeof(double*));
        }
        kernel = get_varh_apo_bmp(kernel_directory,dimension);
    
    
        
        //elegxos an einai .bmp eikona
    if(!strstr(eikona,".bmp")){
        printf("h trith parametros den einai eikona bmp !!!\n");
        exit(1);
    }
        //==================================================================
        //==================================================================
        //metrisi xronou
    clock_gettime (CLOCK_MONOTONIC, &start);
        //==================================================================
        //==================================================================
        
    BMPImage_t* bmp = ReadBMP(eikona);
    int32_t WIDTH = bmp->header.width_px;
    int32_t HEIGHT = bmp->header.height_px;

    MPI_Init(&argc,&argv);

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    
    
    //diadikasia gia grayscaling
    int h=0,w=0,kx=0,ky=0;
    int color_r=0,color_g=0,color_b=0;
        
    
        //gia height
    
    int chunksize_height = HEIGHT / size; //chunksize
    
    

    int start_row_height = rank * chunksize_height;
    int end_row_height = (rank+1)*chunksize_height;

    
     
    BMPImage_t* local_bmp;
    BMPImage_t* teliko_bmp;
    
    
    local_bmp =malloc(sizeof(BMPImage_t));
    local_bmp->header=bmp->header;
    local_bmp->data = calloc(bmp->header.image_size_bytes, sizeof(uint8_t));
    
    teliko_bmp = malloc(sizeof(BMPImage_t));
    teliko_bmp->header = bmp->header;
    teliko_bmp->data = calloc(bmp->header.image_size_bytes, sizeof(uint8_t));

    
    
    
    int array_local_me_pixels_R[chunksize_height][WIDTH];
    int array_local_me_pixels_G[chunksize_height][WIDTH];
    int array_local_me_pixels_B[chunksize_height][WIDTH];

    int telika_local_me_pixels_R[HEIGHT][WIDTH];
    int telika_local_me_pixels_G[HEIGHT][WIDTH];
    int telika_local_me_pixels_B[HEIGHT][WIDTH];

    
    if(rank == 0){
    clock_gettime (CLOCK_MONOTONIC, &start);
    }

    for(h=start_row_height; h<end_row_height && h<HEIGHT ; h++){
        for(w=0; w<WIDTH; w++){
            color_r=0;
            color_g=0;
            color_b=0;

            for(kx=0; kx<dimension; kx++){
                for(ky=0; ky<dimension;ky++){
                    int a=w+kx-1;
                    int b=h+ky+1;
                    if( a<0 ) { a=0 ;}
                    if( a>= WIDTH) { a=WIDTH-1;}
                    if( b< 0 ) {b=0;}
                    if( b>= HEIGHT) { b=HEIGHT-1;}
   
//h for  sthn kx kai ky kanonika prepei na paei apo -1 ws 1 gia na ksekianw apo to mesaio stoixeio
                   // RGB_t color_pix = GetPixel(bmp,a, b);
                    RGB_t color_pix = GetPixel(bmp,a, b);
                    color_r += color_pix.r * kernel[kx][ky] * coeff;
                    color_g += color_pix.g * kernel[kx][ky] * coeff;
                    color_b += color_pix.b * kernel[kx][ky] * coeff;
                    }
            }
                array_local_me_pixels_R[h-start_row_height][w]=color_r;
                array_local_me_pixels_G[h-start_row_height][w]=color_g;
                array_local_me_pixels_B[h-start_row_height][w]=color_b;
        
            }
    }

    
    MPI_Gather(array_local_me_pixels_R, WIDTH*chunksize_height ,MPI_INT ,telika_local_me_pixels_R,  WIDTH*chunksize_height  ,MPI_INT ,0, MPI_COMM_WORLD);
    MPI_Gather(array_local_me_pixels_G, WIDTH*chunksize_height ,MPI_INT ,telika_local_me_pixels_G,  WIDTH*chunksize_height  ,MPI_INT ,0, MPI_COMM_WORLD);
    MPI_Gather(array_local_me_pixels_B, WIDTH*chunksize_height ,MPI_INT ,telika_local_me_pixels_B,  WIDTH*chunksize_height  ,MPI_INT ,0, MPI_COMM_WORLD);

    
    if(rank==0){
        
        teliko_bmp = array_To_bmp(HEIGHT ,WIDTH,telika_local_me_pixels_R,telika_local_me_pixels_G,telika_local_me_pixels_B,bmp);
        
        if(metatropi == 0 ){
            SaveBMP(teliko_bmp,"output_mpi.bmp");
            DestroyBMP(bmp); // apo thn vivliothiki ths bmp
            DestroyBMP(local_bmp);
            DestroyBMP(teliko_bmp);
        }
            //horizontal flip
        else if(metatropi == 1){
            BMPImage_t* gia_allagi =malloc(sizeof(BMPImage_t));
            gia_allagi->header=bmp->header;
            gia_allagi = horizontal_flip_bmp(teliko_bmp, HEIGHT, WIDTH);
            

            SaveBMP(gia_allagi,"output_mpi.bmp");
            DestroyBMP(bmp);
            DestroyBMP(local_bmp);
            DestroyBMP(teliko_bmp);
            DestroyBMP(gia_allagi);
        }
            
            //vertical flip
        else if(metatropi == 2){
            BMPImage_t* gia_allagi =malloc(sizeof(BMPImage_t));
            gia_allagi->header=bmp->header;
            gia_allagi = vertical_flip_bmp(teliko_bmp, HEIGHT, WIDTH);
                
            SaveBMP(gia_allagi,"output_mpi.bmp");
            DestroyBMP(gia_allagi);
            DestroyBMP(bmp); // apo thn vivliothiki ths bmp
            DestroyBMP(local_bmp);
            DestroyBMP(teliko_bmp);
        }
        //=============================================================================
        //=============================================================================
        //              TELOS EURESIS XRONOU
        clock_gettime(CLOCK_MONOTONIC, &end);
        const int DAS_NANO_SECONDS_IN_SEC = 1000000000;
        long timeElapsed_s = end.tv_sec - start.tv_sec;
        long timeElapsed_n = end.tv_nsec - start.tv_nsec;
            //If we have a negative number in timeElapsed_n , borrow
        if ( timeElapsed_n < 0 ) {timeElapsed_n = DAS_NANO_SECONDS_IN_SEC + timeElapsed_n; timeElapsed_s--;}
        printf("Time: %ld.%09ld secs \n",timeElapsed_s,timeElapsed_n);
        //=============================================================================
        //=============================================================================
        //  free ton metavlito pinaka me ta varh
        
        free_2d_malloced_array(kernel,dimension);
        
    }

    MPI_Finalize();

        
        
    return 0;
}

//=====================================================================
//sunartisi gia grayscaling eikonas
//=====================================================================
BMPImage_t* vertical_flip_bmp(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH){
    BMPImage_t* eikona2 =malloc(sizeof(BMPImage_t));
    eikona2->header=input_eikona->header;
    eikona2->data = calloc(input_eikona->header.image_size_bytes, sizeof(uint8_t));
    
    int w,h;
    for(h=0; h<HEIGHT; h++)
    {
        for(w=0; w<WIDTH; w++)
        {
            RGB_t pixel = GetPixel(input_eikona,w,HEIGHT-h-1);
            SetPixel(eikona2, w, h, pixel);
        }
    }
    
    input_eikona=eikona2;
    return input_eikona;
}



//sunartisi gia flip eikonas se horizontal
BMPImage_t* horizontal_flip_bmp(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH){
    BMPImage_t* eikona2 =malloc(sizeof(BMPImage_t));
    eikona2->header=input_eikona->header;
    eikona2->data = calloc(input_eikona->header.image_size_bytes, sizeof(uint8_t));

    int h,w;
    for(h=0; h<HEIGHT; h++)
    {
        for(w=0; w<WIDTH; w++)
        {
            RGB_t pixel = GetPixel(input_eikona,WIDTH-w-1,h);
           
            
            SetPixel(eikona2, w, h, pixel);
        }
    }
    input_eikona=eikona2;
    return input_eikona;
}


BMPImage_t* grayscale(BMPImage_t* input_eikona,int32_t HEIGHT,int32_t WIDTH,double **kernel,double coeff,int dimension_varwn){
//    display_2d_array(kernel, dimension_varwn);
    BMPImage_t* local_bmp;
    local_bmp =malloc(sizeof(BMPImage_t));
    local_bmp->header=input_eikona->header;
    local_bmp->data = calloc(input_eikona->header.image_size_bytes, sizeof(uint8_t));
    
    
    int h=0,w=0,kx=0,ky=0;
    
    for(h=0; h<HEIGHT; h++)
    {
        for(w=0; w<WIDTH; w++)
        {
            int color_r=0;
            int color_g=0;
            int color_b=0;

            for(kx=0; kx<dimension_varwn; kx++)
            {
                for(ky=0; ky<dimension_varwn;ky++)
                {
                    int a=w+kx-1;
                    int b=h+ky+1;
                    if( a<0 ) { a=0 ;}
                    if( a>= WIDTH) { a=WIDTH-1;}
                    if( b< 0 ) {b=0;}
                    if( b>= HEIGHT) { b=HEIGHT-1;}


//h for  sthn kx kai ky kanonika prepei na paei apo -1 ws 1 gia na ksekianw apo to mesaio stoixeio
                    RGB_t color_pix = GetPixel(input_eikona,a, b);
                    color_r += color_pix.r * kernel[kx][ky] * coeff;
                    color_g += color_pix.g * kernel[kx][ky] * coeff;
                    color_b += color_pix.b * kernel[kx][ky] * coeff;
                }
            }
//prepei na ginei douleia stis for gia na apofygw ta errors oson afora tous ypologismous gia th maska, na mpoun oria gia th maska

            RGB_t color;
            color.r = color_r;
            color.g = color_g;
            color.b = color_b;

            SetPixel(local_bmp, w, h, color);
        }
    }

    
    
    return local_bmp;
}


//=====================================================================
//extra sunartiseis
//=====================================================================

//euresh column kai row
int euresh_diastashs_txt_varwn(char *filename){
    char buff[BUFSIZ]={0},temp_char;
    int cols_num=0,rows_num=0;
    char *p;
    
    FILE *file = fopen(filename, "r");
    if(file==0){
        fprintf(stderr,"failed to open txt\n");
        exit(-1);
     }

    //euresh arithmwn column sto txt
    //====================================
    if(NULL==fgets(buff, BUFSIZ, file))
          exit(-1);
       rewind(file);
       for(p=buff;NULL!=strtok(p, " \t\n");p=NULL)
           ++cols_num;
    //====================================
    //euresh arithmwn rows sto txt
    temp_char = getc(file);
  
    while (temp_char != EOF)
    {
            if (temp_char== '\n')
             rows_num += 1;
           temp_char = getc(file);
    }
    //printf("rows: %d\n",rows_num);
    
    if(cols_num!=rows_num){
        printf("O ARITHMOS DIASTASEWN TOU PINAKA VARWN DEN EINAI MORFHS NxN !\n");
        exit(-1);
    }
    
    fclose(file);
    return cols_num;
}

//euresh varwn me xrisi tis sunartiseis pou vriskei tis diastaseis
double **get_varh_apo_bmp(char *filename,int dimension){
    int i,j;
 
    
    FILE *file = fopen(filename, "r");
    if(file==0){
        fprintf(stderr,"failed to open txt\n");
        exit(-1);
     }

    double **varh_bmp_eikonas_fun = (double**)malloc(dimension*sizeof(double*));
    for(i=0;i<dimension;i++){
//        varh_bmp_eikonas_fun[i] =  (double*)malloc(dimension*sizeof(double*));
        varh_bmp_eikonas_fun[i] =  (double*)malloc(dimension*sizeof(double));
    }
    
    for(i = 0; i < dimension; i++) {
        for(j = 0; j < dimension; j++) {
              char s[256] = "0";
              fscanf(file, " %s", s);
              varh_bmp_eikonas_fun[i][j] = atof(s);
          }
      }
    fclose(file);
    
//    printf("PINAKAS VARWN :\n");
//    for(i=0;i<dimension;i++){
//        for(j=0;j<dimension;j++){
//            printf("%.2f   ",varh_bmp_eikonas_fun[i][j]);
//        }
//        printf("\n");
//    }
    
    return varh_bmp_eikonas_fun;
}

//free ton 2d malloc pinaka
void free_2d_malloced_array(double **array,int dimension){
    int i;
    
    for(i=0;i<dimension;i++){
            free(array[i]);
    }
    free(array);
}

//ektupwsh 2d pinaka (gia ta varh)
void display_2d_array(int **array,int dimension){
        int i,j;
    for( i=0;i<dimension;i++){
            for(j=0;j<dimension;j++){
                printf("[%d]",array[i][j]);
            }
        printf("\n");
    }
}


BMPImage_t* array_To_bmp(int Height,int Width,int array_local_me_pixels_R[Height][Width],int array_local_me_pixels_G[Height][Width],int array_local_me_pixels_B[Height][Width],BMPImage_t* bmpp){
    int h,w;
    
    BMPImage_t* local_bmp =malloc(sizeof(BMPImage_t));
    local_bmp->header=bmpp->header;
    local_bmp->data = calloc(bmpp->header.image_size_bytes, sizeof(uint8_t));
    
    for(h=0;h<Height;h++){
        for(w=0;w<Width;w++){
            
            
            RGB_t pixel;
            pixel.r  = array_local_me_pixels_R[h][w];
            pixel.g  = array_local_me_pixels_G[h][w];
            pixel.b  = array_local_me_pixels_B[h][w];
            SetPixel(local_bmp, w, h, pixel);
        }
    }
    return local_bmp;
}





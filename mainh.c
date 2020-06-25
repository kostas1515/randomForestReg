#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#include <dirent.h>
#include <time.h>
#include <math.h>
#define FILE_NUMBER 2//default 52
#define TEST_FILE_NUMBER 1
#include <limits.h> 


struct Stack { 
    int top; 
    unsigned capacity; 
    int* array; 
}; 
  
// function to create a stack of given capacity. It initializes size of 
// stack as 0 
struct Stack* createStack(unsigned capacity) 
{ 
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack)); 
    stack->capacity = capacity; 
    stack->top = -1; 
    stack->array = (int*)malloc(stack->capacity * sizeof(int)); 
    return stack;
}

int EndsWithCsv( char *string )
{
    //returns 0 if file ends with csv
  string = strrchr(string, '.');

  if( string != NULL )
  {
    return( strcmp(string, ".csv") );
  }

  return( -1 );
}

int num_col(char* line)
{
    //counts the number of columns aka feature names
    const char s[2] = ",";
    char *token;

    /* get the first token */
    token = strtok(line, s);
    int i=0;

    /* walk through other tokens */
    while( token != NULL ) {
      i=i+1;
      token = strtok(NULL, s);
    }
    return i;
}

/* Gets the numbers of rows (total) and columns of all files in the "path" directory */
int* getNumbRowsColumns(char *path, char **csvs){
    char temp_path[100];// this is just a helper path
    strcpy(temp_path,path);

    struct dirent *de;  // Pointer for directory entry 
    int csv=0; //counter for csv
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(path); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
  
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    while ((de = readdir(dr)) != NULL)
    {
        //printf("%s\n",de->d_name );
        if (EndsWithCsv(de->d_name)==0)
        {
            strcpy(csvs[csv], de->d_name);
            csv=csv+1;
        }
        
    }
    // FILE *fp = fopen(de->d_name, "r");
    closedir(dr);

    // FILE *fp = fopen(temp_path, "r"); // FOR LINUX
    FILE *fp; //for WINDOWS 
    char buf[3072]; // this is a buffer that contains the line from fgets
    int k=0;
    int col_num=0;
    int row_num=0;
    int TOTAL_NUM_ROW=0;


    //##############the first opening of all the csvs is to get the sizes of columns and rows############################


    for (int i = 0; i < FILE_NUMBER; ++i)//52 are the csvs couldn't make it more generic
    {
        strcat(temp_path,csvs[i]); //copies the file to path, in order to read csv later
        //fp = freopen(temp_path, "r",fp);
        fp=fopen(temp_path,"r");
        //printf("%s\n",temp_path );
        strcpy(temp_path,path);
        while (fgets(buf, 3072, fp))
        {
            if (row_num==0)
            {
                char* tmp = strdup(buf);
                col_num=num_col(tmp);
            }
            row_num++;
        }


        TOTAL_NUM_ROW=TOTAL_NUM_ROW+row_num;
        row_num=0;
        fclose(fp);
        
    }

    int* numRowsColumns = malloc(2*sizeof(int));
    numRowsColumns[0] = TOTAL_NUM_ROW;
    numRowsColumns[1] = col_num;

    return numRowsColumns;
}

void getrow(char* line,char feature_names[][50])
{
    //breaks the line into tokens and returns the feature names
    const char s[2] = ",";
    char *token;

    /* get the first token */
    token = strtok(line, s);
    int i=0;

    /* walk through other tokens */
    while( token != NULL ) {
      strcpy(feature_names[i], token);
      i=i+1;
      token = strtok(NULL, s);
    }
}

void fix_id(char *array)
{
//removes the E from area_id and stores it to data as float
    for (int i = 0; i < NELEMS(array); ++i)
    {
        /* code */
        array[i]=array[i+1];
    }
    for (int i = NELEMS(array)-1; i >0; --i)
    {
        /* code */
        array[i]=array[i-1];
    }
}

void get_data(char* path, int col_num, char **csvs, float** data)
{
    char temp_path[100];
    strcpy(temp_path,path);
    FILE *fp; //for WINDOWS
    //FILE *fp = fopen(temp_path, "r"); // FOR LINUX 
    char buf[3072]; // this is a buffer that contains the line from fgets
    int k=0;
    char line[col_num][50];       //initialize line, it should have same length as feature names [col_name][50], 50 is just a string size number
    char col_names[col_num][50];  
    for (int i = 0; i < FILE_NUMBER; ++i)
    {
        /* itearate through csvs */
        strcat(temp_path,csvs[i]); //add file to path
        //freopen(temp_path, "r",fp); //LINUX
        fp=fopen(temp_path,"r"); //WINDOWS
        strcpy(temp_path,path); //restore absolute path
        

        while (fgets(buf, 3072, fp))
        {
            char* tmp = strdup(buf);
            getrow(tmp,line);//puts temp to line
            if (k==0) //first row is col_names
            {
                for (int c = 0; c < NELEMS(line); ++c)
                {
                    strcpy( col_names[c], line[c]); //just copies the first line to col_names
                }
            }
            else
            {
                for (int c = 0; c < NELEMS(line); ++c)
                {
                    if (c==0)
                    {
                        fix_id(line[c]); //fixes the first column, aka area_id, makes it numerical string without the E
                        
                    }
                    data[k-1][c]=atof(line[c]);// starts from k-1, because for k==0, we have the first row which is the feature names
                }
            }
            k=k+1;
        }  
    }
    fclose(fp);
}
  
// Stack is full when top is equal to the last index 
int isFull(struct Stack* stack) 
{ 
    return stack->top == stack->capacity - 1; 
} 
  
// Stack is empty when top is equal to -1 
int isEmpty(struct Stack* stack) 
{ 
    return stack->top == -1; 
} 
  
// Function to add an item to stack.  It increases top by 1 
void push(struct Stack* stack, int item) 
{ 
    if (isFull(stack)) 
        return; 
    stack->array[++stack->top] = item;
    // printf("%d pushed to stack\n", item);
} 
  
// Function to remove an item from stack.  It decreases top by 1 
int pop(struct Stack* stack) 
{ 
    if (isEmpty(stack)) 
        return INT_MIN; 
    return stack->array[stack->top--]; 
} 
  
// Function to return the top from stack without removing it 
int peek(struct Stack* stack) 
{ 
    if (isEmpty(stack)) 
        return INT_MIN; 
    return stack->array[stack->top]; 
} 

void printAr1(int *myArr, int length)
{
    for(int i=0; i<length; i++)
    {
        printf("%d  ", myArr[i]);
    }
}

void split(int *size_matrix,float **child1, float **child2, float **parent)
{
    int parent_i = size_matrix[0];
    int parent_j = size_matrix[1];

    int child1_i = size_matrix[4];
    int child1_j = size_matrix[1];

    int child2_i = size_matrix[5];
    int child2_j = size_matrix[1];

    int c1i=0;
    int c1j=0;
    int c2i=0;
    int c2j=0;
    int best_feature=size_matrix[2];
    int best_threshold =size_matrix[3];



    for (int j=0;j<parent_j;j++)
    {
        c1i=0;
        c2i=0;
        for (int i=0;i<parent_i;i++)
        {
            if (parent[i][best_feature]<parent[best_threshold][best_feature])//true for {0,1,2}
            {
                child1[c1i][c1j]=parent[i][j];
                c1i++;
            }
            else
            {
                child2[c2i][c2j]=parent[i][j];
                c2i++;
            }
        }
        c1j++;
        c2j++;
    }

    size_matrix[4]=c1i;
    size_matrix[5]=c2i;

}

/*Calculate the numebr of rows for each children before the split*/
void get_split_childsizes(int *util_matrix, float **parent)
{
    int num_rows = util_matrix[0];//(total) number of rows
    int num_tree_columns = util_matrix[1];//number of colums in each tree

    int size_left_child=0;
    int size_right_child=0;


    for (int i=0;i<num_rows;i++)
    {
        if (parent[i][util_matrix[2]]<parent[util_matrix[3]][util_matrix[2]])//true for {0,1,2}
        {
            size_left_child++;
        }
        else
        {
            size_right_child++;
        }
    }
    util_matrix[4] = size_left_child;
    util_matrix[5] = size_right_child;
}

void update_utility_matrix(int *size_matrix,int temp_numel)
{
    size_matrix[0]= temp_numel;
}

void get_best_threshold(int *size_matrix, float **parent)
{
    /*
    updates utilities_matrix last position, which is the row index of the best threshold where the split must happen.
    the two submatrices are [0..,k] , [k+1,..,row_number] 
    */

    int parent_i = size_matrix[0];
    int parent_j = size_matrix[1];
    double current_threshold=0;
    int best_threshold=0;
    int best_descriptor=0;
    double min_se=pow(10,9);
    double se = 0;
    int counter=0;
    double sumLeft = 0;
    double sumRight = 0;
    double avgTarget =0;

    for (counter = 0; counter < parent_i; ++counter)
    {
        /* code */
        avgTarget=avgTarget+parent[counter][parent_j-1];
    }
    avgTarget=avgTarget/counter;
    // printf("avg target is %f\n",avgTarget );
    
    for(int m=0;m<parent_j-1;m++)
    {
        // printf("mse is %f\n",min_se );
        for(int k=0; k<parent_i; k++)
        {
            current_threshold = parent[k][m];

            counter =0;
            sumLeft=0;
            sumRight=0;
            se=0;
            for(int i=0; i<parent_i; i++)
            {
                if(parent[i][m]<= current_threshold)
                {
                    sumLeft += pow(parent[i][parent_j-1]-avgTarget,2);//target value
                    counter++;
                }
                else
                {
                    sumRight += pow(parent[i][parent_j-1]-avgTarget,2);//target value 
                }
            }
            // printf("current sumRight is %f\n", sumRight);
            //Compute the average

            sumLeft=(counter*sumLeft)/parent_i;
            if (counter!=parent_i)
            {
                /* code */
                sumRight=sumRight*(parent_i-counter)/parent_i;
            }

            se=(sumRight+sumLeft)/2;
            if(se<min_se)
            {
                min_se=se;
                best_threshold = k;
                best_descriptor=m;
                // printf("the best is %d\n",parent_i-counter );
            }
        }
    }

    size_matrix[3]=best_threshold;
    size_matrix[2]=best_descriptor;

}

/**
 * This function normalises the elements of a matirx column-wise 
**/
void normalise(float **matrix, int rows, int cols)
{
    for(int j=0; j<cols; ++j)
    {
        float mean =0;
        for(int i=0; i<rows; ++i)
        {
            mean += matrix[i][j];
        }
        mean = mean/rows;

        float sdev=0;
        for(int i=0; i<rows; i++)
        {
            sdev += pow((matrix[i][j]-mean),2);
        }
        sdev = sqrt(sdev/(rows-1));

        if(sdev==0)
        {
            //printf("%f",mean);
            sdev=1; 
        }

        for(int i=0; i<rows; i++)
        {
            matrix[i][j]=(matrix[i][j]-mean)/sdev;
        }
    }
}


/**
 * This function calculates which feature has the best mse, to decide the next split as a
 * decision tree is created and a also decides a proper threshold.
 * This functions works along with a vector of integers called size_matrix which is defined as
 * size_matrix[4] = [num_row_of_the parent,num_col_of_the_parent,best_feature,best_threshold]
 * This functions stores in matrix[2] the best feature (number of column) and matrix[3] the
 * best threshold.
**/
void get_best_descriptor(int *size_matrix, float **parent)
{

    int parent_i = size_matrix[0];
    int parent_j = size_matrix[1]-1; //minus 1 because the last column is the target and must be excluded

    int k=0; //check for all feature colms
    double min_mse=pow(10,7); //a very big mse to start with
    double average_descripor=0;
    double se=0;
    int best_descriptor;

    for (int j =0;j<parent_j;j++)
    {
        //calculate mean squere error
        average_descripor=0;
        for (int i = 0; i < parent_i; ++i)
        {
            /* sum all features */
            average_descripor=average_descripor+parent[i][j];
        }
        //get avrg
        average_descripor=average_descripor/parent_i;
        for (int i = 0; i < parent_i; ++i)
        {
            /* sum  */
            se=se+pow((average_descripor-parent[i][parent_j]),2);
        }
        se=se/parent_i;
        
        if (se<min_mse)
        {
            min_mse=se;
            best_descriptor=j;
            //printf("%d\n",j );
            se=0;       /* code */
        }
        else
        {
            se=0;
        }

    }
    // printf("%f\n", min_mse );
    size_matrix[2]=best_descriptor;
}


/**
 * Create Bag Of Features
 * This is a function to select randomly features for each tree of the Random Forest.
 * This is a functions that takes as an input the number of features in total, 
 * the number of features that the algorithm selects randomly to create the root of each tree
 * the number of column that is the target (the feture that we are trying to estimate in the regression)
 * this is an input of this function to ensure that the target is not selected in the random selection.
 * and an integer number called seed that creates some noise and helps in the random selection.
 * Returns an array with randomly selected numbers from 0 to cols. The 
 * returned array has size Nfeatures.
**/
void createBOF(int cols, int Nfeatures,int target, int seed, int *bof)
{
    srand(time(NULL));   // Initialization, should only be called once.
    int r = 0;
    int exclude_list[23]={0,18,26,34,42,50,58,66,67,74,75,76,77,82,90,98,106,114,122,123,130,131,132};

    int flag=1;
    int array1[cols];

    for(int i=0; i<cols; i++)
    {
        array1[i] = i; 
    }

    int i=0;
    if(seed==0)
    {
        seed = rand()%5;
    }

    while(i<Nfeatures-1)
    {
        r = ((rand()%11)*seed + (rand()%13)*seed + (rand()%109)*seed + (rand()%71)*seed + (rand()%203)*seed ) % (cols-i);
        flag=1;
        for (int e = 0; e < 23; ++e)
        {
            /* code */
            flag=flag&&(r!=exclude_list[e]);
        }
        if ((r!=target)&&flag)
        {
            bof[i] = array1[r];
            array1[r] = array1[cols-1-i];
            i++;
        }
    }
    bof[Nfeatures-1] = target;

    //printf("\n-------------------------------------------------------------------------------------------\n");

}
/**
 * This functions fills a submatrix that is in essence the root of a tree of the random forest. 
 * As input it takes the whole data set which has "bigCols" features and then it selects randomly 
 * "Nfeatures" features to fill the smallMatr.
**/
void fillBof(int rows, int bigCols, int Nfeatures,float **bigMatr, float **smallMatr,int *bofAr,int target,int seed)
{
    createBOF(bigCols,Nfeatures,target,seed,bofAr);

    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<Nfeatures; j++)
        {
            smallMatr[i][j] = bigMatr[i][bofAr[j]];
        }
    } 
}

/**
 * This function copies the target feature from the column col_name of matrix data 
 * to an array y that has length length (the first argument)
**/
void get_target(int length,int col_name,float **data,float *y)
{
    for(int i=0; i<length; i++)
    {
        y[i]=data[i][col_name];
    }
}

// A function to print a matrix with integers
void printMatr(int rows, int cols, int **Matr)
{
    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            printf("%d  ", Matr[i][j]);
        }
        printf("\n");
    }
    printf("------------------------------------------------------------------------------------------------\n");
}

// A function to print a matrix with floats
void printMatrF(int rows, int cols, float **Matr)
{
    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            printf("%f  ", Matr[i][j]);
        }
        printf("\n");
    }
    printf("-----------------------------------------------------------------------------------------------\n");
}


int main(void) 
{

    // char path[100]="D:/Work/PhD/HartreeTraining/Hartree Assignment/data/";// this path should be the folder of data
    // char path[100]="/home/konsa/Downloads/hartree_data/";
    char path[100]="C:\\Users\\theot\\eclipse-workspaceC\\VSC\\Hartree\\Hartree_Data\\";
    int* num_rows_columns = malloc(2*sizeof(int)); //holds number of rows and columns
    
    char **csvs = malloc(FILE_NUMBER * sizeof(char *));
    for (int i=0; i < FILE_NUMBER; i++)
        csvs[i] = malloc((50)*sizeof(float));

    //Go trough all files to check how many rows and columns of data we'll need to allocate
    num_rows_columns = getNumbRowsColumns(path, csvs);
    int total_num_rows = num_rows_columns[0];
    int num_columns = num_rows_columns[1];

    //############################### READING THE DATA FROM CSV FILES ###########################################


    int TOTAL_NUM_ROW=total_num_rows-1;
    printf("Total number of rows is:%d\n",TOTAL_NUM_ROW );
    printf("Total number of columns is:%d\n",num_columns );


    //################### main data procedure#################

    //initialize feature_name list
    float **train_data = malloc(TOTAL_NUM_ROW * sizeof(float *)); //initialize data 2d array
    for (int i=0; i < TOTAL_NUM_ROW; i++)
        train_data[i] = malloc((num_columns)*sizeof(float));

    float *target_y=malloc(TOTAL_NUM_ROW*sizeof(float));

    get_data(path, num_columns, csvs, train_data);
    
 //printMatr(TOTAL_NUM_ROW,col_num,data);

 //############################### CREATING THE RANDOM FOREST ###########################################

    //########################     Some Initialisation    ###############################################
    int target_feature = 193; //Target feautr is the population in 193th col
    int tree_size=round(sqrt(num_columns))+1; // The number of feature that the root of each tree gets
    int num_of_trees=5;  //The number of trees
    int depth = 4;
    int no_Of_nodes = (int)pow(2,depth)-1;
    struct Stack* node_stack=createStack(no_Of_nodes);
    struct Stack* level_stuck=createStack(no_Of_nodes);
    struct Stack* rt_size=createStack(no_Of_nodes);

    int root=(no_Of_nodes/2);
    int current_root=0;
    int level=1;
    int splitable=1;
    int left_node=0;
    int right_node=0;
    int parent_size=0;
    float **temp_child_data;
    int **bofArrays = malloc(num_of_trees * sizeof(int *));
    int bofArray[tree_size];
    int minimum_leaf_size=16;
    int left_child_size_flag=1;
    int right_child_size_flag=1;
    printf("The number of nodes is %d\n",no_Of_nodes);



    int num_rows ;//(total) number of rows
    int num_tree_columns ;//number of colums in each tree

    int size_left_child=0;
    int size_right_child=0;



    /**
     * Memory allocation for the forest.
     * Each node is a 2D matrix thus each tree is a 3D matrix and the forest is a 4D matrix.
     **/

    float ****forest = malloc(num_of_trees * sizeof(float ***));
    for (int f = 0; f < num_of_trees; ++f)
    {
        forest[f]=malloc(no_Of_nodes * sizeof(float **));
    }


    //root data matrix.
    float ***roots_temp = malloc(num_of_trees * sizeof(float **));
    for (int k = 0; k < num_of_trees; ++k)
    {
        roots_temp[k]=malloc(TOTAL_NUM_ROW*sizeof(float *));
        for (int i=0; i < TOTAL_NUM_ROW; i++)
        {
            roots_temp[k][i] = malloc(tree_size*sizeof(float));
        }
    }


    for (int i = 0; i < num_of_trees; ++i)
    {
        /* fill first layer */
        forest[i][root]=roots_temp[i];
    }



    //Normalise the data table
    normalise(train_data,TOTAL_NUM_ROW,num_columns);

    //Copy the target column from data
    get_target( TOTAL_NUM_ROW,target_feature,train_data,target_y);

    //fill the ROOOOOTTT of the tree, it will be the middle index of the node array
    

    int **utilities_matrix = malloc(num_of_trees * sizeof(int *));
    for (int i=0; i < num_of_trees; i++)
    {
            utilities_matrix[i] = malloc(6 * sizeof(int));
    }

    //Initialise the above matrix.
    for(int i=0; i<num_of_trees; i++)
    {
        utilities_matrix[i][0]=TOTAL_NUM_ROW;
        utilities_matrix[i][1]=tree_size;
        utilities_matrix[i][2]=0;
        utilities_matrix[i][3]=0;
        utilities_matrix[i][4]=0;
        utilities_matrix[i][5]=0;
    }

    for (int i = 0; i < num_of_trees; ++i)
    {   
        fillBof(TOTAL_NUM_ROW,num_columns,tree_size,train_data,forest[i][root],bofArray,target_feature,i+3);
        bofArrays[i] = bofArray;
    }
    // feat_thres is a 3-d matrix [num_of_tress][num_of_nodes][best_feature,float threshold]
    float ***feat_thres = malloc(num_of_trees * sizeof(float **));
    for (int j = 0; j < num_of_trees; ++j)
    {
        feat_thres[j]=malloc(no_Of_nodes * sizeof(float *));
        for (int i=0; i < no_Of_nodes; i++)
            feat_thres[j][i] = malloc((2)*sizeof(float));
    }

    // printMatrF(TOTAL_NUM_ROW,tree_size,forest[9][root]);
    // printMatrF(TOTAL_NUM_ROW,tree_size,forest[8][root]);
    // printMatrF(TOTAL_NUM_ROW,tree_size,forest[7][root]);

    for (int fi = 0; fi < num_of_trees; ++fi)
    {
        /* forest iteration */
        level=1;

        push(node_stack,root);//3
        push(level_stuck,level);//1
        push(rt_size,utilities_matrix[fi][0]);
        printf("\nTree %d initial size is:%d  ",fi,utilities_matrix[fi][0]);
        splitable=1;


        while(!isEmpty(node_stack))
        {
            current_root=pop(node_stack);//3
            printf("current root = %d ", current_root);
            parent_size=pop(rt_size);
            printf(" Current parent size = %d \n", parent_size);
            level=pop(level_stuck);

            update_utility_matrix(utilities_matrix[fi],parent_size);//
            

            // get_best_descriptor(utilities_matrix[fi], forest[fi][current_root]);
            get_best_threshold(utilities_matrix[fi], forest[fi][current_root]);

            
            get_split_childsizes(utilities_matrix[fi],forest[fi][current_root]);

            // utilities_matrix[fi][5]=33;
            num_rows = utilities_matrix[fi][0];//(total) number of rows

            

            printf("left size is: %d", utilities_matrix[fi][4]);
            printf("  right size is: %d\n", utilities_matrix[fi][5]);
           
            // printf("%f\n",forest[fi][current_root][utilities_matrix[fi][3]][utilities_matrix[fi][2]] );

            left_child_size_flag=0;
            right_child_size_flag=0;

            feat_thres[fi][current_root][0]=forest[fi][current_root][utilities_matrix[fi][3]][utilities_matrix[fi][2]];
            printf("the numeric threshold is %f\n",feat_thres[fi][current_root][0]);
            feat_thres[fi][current_root][1]= bofArrays[fi][utilities_matrix[fi][3]];
            printAr1(utilities_matrix[fi],6);
            // printAr1(utilities_matrix[fi],6);


            // printMatr(num_of_trees,6,utilities_matrix);
            
            // printMatr(num_of_trees,4,utilities_matrix);
            // printMatrF(parent_size,tree_size,forest[fi][current_root]);

                //todo get numeric threshold
            if(parent_size<minimum_leaf_size)
            {
                splitable=0;
                printf("the number of elems is:%d\n",parent_size );
            }
            if(current_root%2!=0 &&splitable) // if splitable 
            {
                left_node=current_root-(int)pow(2,depth-level-1);//1,0
                right_node=current_root+(int)pow(2,depth-level-1);//5,2

                if ((level+1)<depth)
                {
                    if(utilities_matrix[fi][5]>minimum_leaf_size)
                    {
                        printf("pushed right\n");
                        push(rt_size,utilities_matrix[fi][5]);//right node
                        push(node_stack,right_node);//11
                        push(level_stuck,level+1);//1+1=2
                    }
                    if(utilities_matrix[fi][4]>minimum_leaf_size)
                    {   
                        printf("pushed left\n");
                        push(rt_size,utilities_matrix[fi][4]);//left node 32
                        push(node_stack,left_node);//3,11
                        push(level_stuck,level+1);//1+1=2
                    }
                }
                if (utilities_matrix[fi][4]>0)
                {
                    /* code */
                    temp_child_data=malloc((utilities_matrix[fi][4])*(sizeof(float *)));
                    for (int i = 0; i < utilities_matrix[fi][4]; ++i)
                    {
                        temp_child_data[i]=malloc(tree_size*(sizeof(float)));
                    }
                    forest[fi][left_node]=temp_child_data;
                    left_child_size_flag=1;
                }

                
                if(utilities_matrix[fi][5]>0)
                {
                    temp_child_data=malloc((utilities_matrix[fi][5])*(sizeof(float *)));
                    for (int i = 0; i < utilities_matrix[fi][5]; ++i)
                    {
                        temp_child_data[i]=malloc(tree_size*(sizeof(float)));
                    }
                    forest[fi][right_node]=temp_child_data;
                    right_child_size_flag=1;
                }


                if(left_child_size_flag&&right_child_size_flag)
                {
                    split(utilities_matrix[fi],forest[fi][left_node],forest[fi][right_node],forest[fi][current_root]);
                }
                else if(left_child_size_flag&&!right_child_size_flag)
                {
                    forest[fi][left_node]=forest[fi][current_root];
                }
                else
                {
                    forest[fi][right_node]=forest[fi][current_root];
                }
            }

            if(current_root%2==0 || !splitable){
                feat_thres[fi][current_root][2] == 1; //can either associate the leaf flag to the threshold value
            }

        }

    }
    //printMatr(num_of_trees,6,utilities_matrix);

    // path[100]="/home/konsa/Downloads/hartree_data/test_sample/";
    char path2[100] = "C:\\Users\\theot\\eclipse-workspaceC\\VSC\\Hartree\\Hartree_Test\\";

    char **test_csvs = malloc(TEST_FILE_NUMBER * sizeof(char *));
    for (int i=0; i < TEST_FILE_NUMBER; i++)
        test_csvs[i] = malloc((50) *sizeof(float));

    //Go trough all files to check how many rows and columns of data we'll need to allocate
    num_rows_columns = getNumbRowsColumns(path2, test_csvs);
    total_num_rows = num_rows_columns[0];
    num_columns = num_rows_columns[1];

    // ############################### READING THE DATA FROM CSV FILES ###########################################


    TOTAL_NUM_ROW=total_num_rows-1;
    printf("Total number of test_rows is:%d\n",TOTAL_NUM_ROW );
    printf("Total number of test_columns is:%d\n",num_columns );


    //################### main data procedure#################

    //initialize feature_name list
    float **test_data = malloc(TOTAL_NUM_ROW * sizeof(float *)); //initialize data 2d array
    for (int i=0; i < TOTAL_NUM_ROW; i++)
        test_data[i] = malloc((num_columns) * sizeof(float));



    get_data(path, num_columns, test_csvs, test_data);
    free(target_y);// free train_target and have it for testing, should we redeclare????!!!


    float prediction_temp=0;

    float avg_mse = 0; 

    int current_node=root;

    normalise(test_data,TOTAL_NUM_ROW,num_columns);

    for (int i = 0; i < TOTAL_NUM_ROW; ++i)
    {
        /* for all data points */
        prediction_temp=0;
        for (int fi = 0; fi < num_of_trees; ++fi)
        {
            /* for all trees */
            current_node =  root;
            level=1;
            while(!(int)feat_thres[fi][current_node][2])
            {
                if(test_data[i][(int)feat_thres[fi][current_node][1]] > feat_thres[fi][current_node][0])
                {
                    current_node += (int)pow(2,depth-level-1);
                    level++;
                }
                else
                {
                    current_node -= (int)pow(2,depth-level-1);
                    level++;
                }
            }
            prediction_temp += feat_thres[fi][current_node][0];

        }
        prediction_temp = prediction_temp/num_of_trees;
        avg_mse += pow(test_data[i][target_feature] - prediction_temp,2);
    }
    avg_mse = avg_mse/TOTAL_NUM_ROW;
    
    printf("The avg MSE is : %f", avg_mse);
}

#ifdef __unix__
    #include <stdio.h>
    #include <math.h>
#endif
# define N 4


void getCofactor(int matrix[N][N], 
                 int temp[N][N], 
                 int p, 
                 int q, 
                 int n);

int determinantOfMatrix(int matrix[N][N], int n);









int main(){


    

    int matrix[N][N] = {{1, 0, 2, -1},
                        {3, 0, 0, 5},
                        {2, 1, 4, -3},
                        {1, 0, 5, 0}};


    printf("Determinant of the matrix is : %d", determinantOfMatrix(matrix, N));
    return 0;


    return 0;

}

/* Recursive function for finding the 
   determinant of matrix. n is current 
   dimension of mat[][]. */
int determinantOfMatrix(int matrix[N][N], int n){


    int idx;

    // Initialize result
    int D = 0;
    
    // To store cofactors
    int temp[N][N]; 
  
    // To store sign multiplier
    int sign = 1;
    
    
       
    //  Base case : if matrix contains 
    // single element
    if (n == 1)
        return matrix[0][0];
 
    // Iterate for each element of 
    // first row
    for (idx=0; idx<n; idx++) 
    {
        // Getting Cofactor of mat[0][f]
        getCofactor(matrix, temp, 0, idx, n);
        
        D += sign * matrix[0][idx] * determinantOfMatrix(temp, n - 1);
 
        // Terms are to be added with alternate sign
        sign = -sign;
    }
 
    return D;
}


void getCofactor(int matrix[N][N], 
                 int temp[N][N], 
                 int p, 
                 int q, 
                 int n){
                 
                 
    // Function to get cofactor of matrix[p][q] 
    // in temp[][]. n is current dimension 
    // of matrix[][]             
                 
    int idx = 0, kdx = 0;
    int row;
    int col;
 
    // Looping for each element of the matrix
    for (row=0; row<n; row++)
    {
        for (col=0; col<n; col++) 
        {
            // Copying into temporary matrix 
            // only those element which are 
            // not in given row and column
            if (row != p && col != q) 
            {
                temp[idx][kdx++] = matrix[row][col];
 
                // Row is filled, so increase row 
                // index and reset col index
                if (kdx == n - 1) 
                {
                    kdx = 0;
                    idx++;
                }
            }
        }
    }
}

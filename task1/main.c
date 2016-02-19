#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, const char *argv[])
{
    int n,i,j,X1,X2,Y1,Y2,b,space;
    scanf("%d", &n);
    int **a=(int**)malloc(n*sizeof(int*));
    for (i=0;i<n;i++){
        a[i]=(int*)malloc(n*sizeof(int));
        for (j=0;j<n;j++){
            a[i][j]=(i+1)*(j+1);
        }
    }
    scanf("%d",&X1);
    while (X1){
    if (X1 != 0){
            scanf("%d%d%d", &Y1,&X2,&Y2);
            b = a[X2][Y2];
            space=1;
            while b(>0){
                b=b/10;
                space=space+1;
            }            
            for(i=X1-1;i<X2;i++){
                for (j=Y1-1;j<Y2;j++){
                    printf("%-*d", space, a[i][j]);
                }
                printf("\n");
            }
        }
    printf("\n");
    scanf ("%d",&X1);
    }
    for (i=0;i<n;i++){
        free(a[i]);
    }
    free(a);
    return 0;
}

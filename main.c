#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

int main(int argc, const char *argv[])
{
    int n,i,j,X1,X2,Y1,Y2,space;
    const int b=5;
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
            space = (int)log10(a[X2][Y2])+2;
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
    getch();
    return 0;
}


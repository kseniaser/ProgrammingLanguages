#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(int argc, const char *argv[])
{
    int n,i,j,X1,X2,Y1,Y2,space;
    scanf("%d", &n);
    int **a=malloc(n*sizeof(int*));
    for (i=0;i<n;i++){
        a[i]=malloc(n*sizeof(int));
        for (j=0;j<n;j++){
            a[i][j]=(i+1)*(j+1);
        }
    }
   char str[20];
   sprintf(str,"%d",a[n-1][n-1]);
   space=strlen(str)+1;
    scanf("%d",&X1);
    while (X1){
            scanf("%d%d%d", &Y1,&X2,&Y2); 
            printf("\n");         
            for(i=X1-1;i<X2;i++){
                for (j=Y1-1;j<Y2;j++){
                    printf("%*d", space, a[i][j]);
                }
                printf("\n");
            }
    scanf ("%d",&X1);
    }
    for (i=0;i<n;i++){
        free(a[i]);
    }
    free(a);
    return 0;
}

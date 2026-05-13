#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lexicographic_sort(const char* a, const char* b) {
    int i = 0;
    while( (a[i] != '\0') && (b[i] != '\0') )
    {
        if (a[i] > b[i])
            return 1;
        else if (a[i] < b[i])
            return 0;
        i++;
    }
    
    if (a[i] != '\0') return 1;
    return 0;
}


int lexicographic_sort_reverse(const char* a, const char* b) {
    int i = 0;
    while( (a[i] != '\0') && (b[i] != '\0') )
    {
        if (a[i] > b[i])
            return 0;
        else if (a[i] < b[i])
            return 1;
        i++;
    }
    
    if (b[i] != '\0') return 1;
    return 0;
}

int sort_by_number_of_distinct_characters(const char* a, const char* b) {
    char distA[26] = {0};
    char distB[26] = {0};
    
    int lenA = strlen(a);
    int lenB = strlen(b);
    
    for(int i = 0; i < lenA; i++)
    {
        if (a[i] >= 'a' && a[i] <= 'z')
        {
            distA[a[i] - 'a'] = 1;
        }
    }
    
    for(int i = 0; i < lenB; i++)
    {
        if (b[i] >= 'a' && b[i] <= 'z')
        {
            distB[b[i] - 'a'] = 1;
        }
    }
    
    int sumA = 0;
    int sumB = 0;
    for (int i = 0; i < 26; i++)
    {
        sumA += distA[i];
        sumB += distB[i];
    }
    
    if (sumA > sumB)
    {
        return 1;
    }
    else if (sumA == sumB) 
    {
        return lexicographic_sort(a, b);
    }
    else 
    {
        return 0;
    }
}

int sort_by_length(const char* a, const char* b) {
    int lenA = strlen(a);
    int lenB = strlen(b);
    
    if (lenA > lenB)
    {
        return 1;
    }
    else if (lenA == lenB)
    {
        return lexicographic_sort(a, b);
    }
    else 
    {
        return 0;
    }
}

void string_sort(char** arr,const int len,int (*cmp_func)(const char* a, const char* b)){
    for (int i = 0; i < len; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            if (cmp_func(arr[i], arr[j]))
            {
                char *temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}


int main() 
{
    int n;
    scanf("%d", &n);
  
    char** arr;
	arr = (char**)malloc(n * sizeof(char*));
  
    for(int i = 0; i < n; i++){
        *(arr + i) = malloc(1024 * sizeof(char));
        scanf("%s", *(arr + i));
        *(arr + i) = realloc(*(arr + i), strlen(*(arr + i)) + 1);
    }
  
    string_sort(arr, n, lexicographic_sort);
    for(int i = 0; i < n; i++)
        printf("%s\n", arr[i]);
    printf("\n");

    string_sort(arr, n, lexicographic_sort_reverse);
    for(int i = 0; i < n; i++)
        printf("%s\n", arr[i]); 
    printf("\n");

    string_sort(arr, n, sort_by_length);
    for(int i = 0; i < n; i++)
        printf("%s\n", arr[i]);    
    printf("\n");

    string_sort(arr, n, sort_by_number_of_distinct_characters);
    for(int i = 0; i < n; i++)
        printf("%s\n", arr[i]); 
    printf("\n");

    return 0;
}
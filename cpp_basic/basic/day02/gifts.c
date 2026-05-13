// Function Description

// Complete the function, marks_summation in the editor below.

// marks_summation has the following parameters:

//     int marks[number_of_students]: the marks for each student
//     int number_of_students: the size of marks[]
//     char gender: either 'g' or 'b'

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

//Complete the following function.

int marks_summation(int* marks, int number_of_students, char gender) {
  //Write your code here.
  int sum = 0;
  if (gender == 'b') {
    for (int i = 0; i < number_of_students; ++i) {
        if ( i % 2 == 0) {
            sum += marks[i];
        }
    }
  }
  else if (gender == 'g') {
    if (number_of_students == 1) {
        return 0;
    }
    for (int i = 0; i < number_of_students; ++i) {
        if ( i % 2 != 0) {
            sum += marks[i];
        }
    }
  }
  return sum;
}

int main() {
    int number_of_students;
    char gender;
    int sum;
  
    scanf("%d", &number_of_students);
    int *marks = (int *) malloc(number_of_students * sizeof (int));
 
    for (int student = 0; student < number_of_students; student++) {
        scanf("%d", (marks + student));
    }
    
    scanf(" %c", &gender);
    sum = marks_summation(marks, number_of_students, gender);
    printf("%d", sum);
    free(marks);
 
    return 0;
}
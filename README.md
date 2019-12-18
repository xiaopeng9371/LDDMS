#==== Prerequisite ====

Java 8
All code is written in java 8. Tested in Eclipse on Windows 10. Make sure Java is istalled on your machine.

#==== Usage ====
## Question 1
This program reads some integers contained in the file Q1_input.txt, where these integers are delimited by space character, Tab character, comma, or newline character. The java script will read the file Q1_input.txt, and calculate the mean, median, and mode of these numbers. 

In the folder of Question 1,
Make sure the absolute path of the input file is correct, you may need to change it in the main function: 
```Java
File file = new File("C:\\Users\\xiaopeng\\eclipse-workspace\\Question1\\src\\Q1_input.txt");
```
To compiler
Run 
$ javac Main.java 
in terminal (Mac or Linux) or command line in Windows, then run 
$ Java Main 
to check the result.

## Question 2
This program computes the edit distance of two strings. Dynamic programming is used where the time complexity of O(mn) where m and n are the length of the two strings.
In the folder of Question 2,
To compiler
Run 
$ javac Main.java 
in terminal (Mac or Linux) or command line in Windows, then run 
$ Java Main 
to check the result.

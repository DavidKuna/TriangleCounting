/* 
 * File:   main.cpp
 * Author: David Kuna
 *
 * Created on 23. listopad 2013, 16:47
 */

#include <iostream>		// std::cout
#include <fstream>		// std::ifstream
#include <vector>		// std::vector
#include <algorithm>    // std::find
#include <sys/time.h>

using namespace std;

/**
 * Zjistí zda řádek obsahuje hledané číslo
 * @param search
 * @param array
 * @return boolean
 */
bool isInArray(unsigned long search, unsigned long *array, bool debug = false){
	if(debug == true){
		cout << "search = " << search << " array[0] = " << array[0] << endl;
	}
	for(unsigned long i = 1; i <= array[0]; i++){
		if(array[i] == search) return true;
	}
	return false;
}

/**
 * Načte datový soubor a naplní strukturu
 * 
 * @param filepath
 * @param rows
 * @return boolean
 */
bool loadGraph(char *filepath, unsigned long **&rows){
	unsigned long tmpNode, node, key, value;
	unsigned int pairs = 0;
	ifstream source (filepath);
	
	if(source.is_open()){
		while (source >> node) {
			pairs++;
			if(pairs % 2 == 1){
				tmpNode = node;
			}else{
				if(tmpNode < node){
					key = tmpNode;
					value = node;
				}else{
					key = node;
					value = tmpNode;
				}
				// Zjistíme zda uz vektor teto prvek obsahuje a pokud ano tak ho přeskočíme
				 if(key != value && !isInArray(value, rows[key])){
					rows[key][0]++;
					rows[key][rows[key][0]] = value;
				 }
			}
			pairs = pairs % 2;
		}
		cout << "done" << endl;
		return true;
	}else{
		cout << "fail" << endl;
		cout << "Unable to open source file (" << filepath << ")" << endl;
	}
	return false;
}

/**
 * Načte datový soubor a naplní strukturu
 * 
 * @param filepath
 * @param rows
 * @return boolean
 */
unsigned long **prepareStructure(unsigned long *columns, unsigned long countOfNodes){
	unsigned long **rows = new unsigned long*[countOfNodes+1];
	for(long r = 0; r <= countOfNodes; r++){
		rows[r] = new unsigned long[columns[r]+1];
		fill_n(rows[r], columns[r]+1, 0);
	}
	cout << "done" << endl;
	return rows;
}

/**
 * Najde vrchol s nejvyšším indexem
 * @param filepath
 * @return int
 */
unsigned long findMax(char *filepath){
	unsigned long max = 0;
	long node;
	ifstream source (filepath);
	if(source.is_open()){
		while (source >> node) {
			if(max < node)	max = node;
		}
		cout << "done" << endl;
		return max;
	}else{
		cout << "fail" << endl;
		cout << "Unable to open source file (" << filepath << ")" << endl;
	}
	cout << "fail" << endl;
	return 0;
}

/**
 * Určí počet sousedů klíčových uzlů
 * @param filepath
 * @param countOfNodes
 * @return array
 */
bool countColumns(char *filepath, unsigned long *columns){
	unsigned long tmpNode, node;
	unsigned int pairs = 0;
	ifstream source (filepath);
	if(source.is_open()){
		while (source >> node) {
			pairs++;
			if(pairs % 2 == 1){
				tmpNode = node;
			}else{
				if(tmpNode < node){
					columns[tmpNode]++;
				}else{
					columns[node]++;
				}
			}
			pairs = pairs % 2;
		}
		cout << "done" << endl;
		return true;
	}else{
		cout << "Unable to open source file (" << filepath << ")" << endl;
	}
	return 0;
}

/**
 * Spočítá trojůhelníky hlavních vrchlů v zadaném rozsahu
 * @param rows
 * @param indexFrom
 * @param indexTo
 * @param triangles
 */
unsigned long countTriangles(unsigned long **&rows, unsigned long countOfNodes){
	struct timeval start, finish;
	unsigned long triangles = 0;
	long mtime, seconds, useconds;
	
	gettimeofday(&start, NULL);
	
	#pragma omp parallel for schedule(dynamic) reduction(+:triangles) shared(rows)
	for(unsigned long i = 0; i < countOfNodes; i++){
		for(unsigned long j = 1; j <= rows[i][0]; j++){
			for(unsigned long k = 1; k <= rows[rows[i][j]][0]; k++){
				if(isInArray(rows[rows[i][j]][k], rows[i])){
					triangles++;
				}
			}
		}
	}
	
	gettimeofday(&finish, NULL);	
	seconds = finish.tv_sec - start.tv_sec;
	useconds = finish.tv_usec - start.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	cout << "done" << endl;
	cout << "--------RESULT---------" << endl;
	cout << "Execution time: " << mtime*0.001 << "s" << endl;
	
	return triangles;
}

/**
 * Vstupní bod programu
 * 
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char** argv) {
	unsigned long countOfNodes, triangles = 0;
	char *filename;	
	
	filename = argv[1];
	countOfNodes = (unsigned long)atoi(argv[2]);
	if(countOfNodes == 0){
		cout << "Nodes counting... ";
		countOfNodes = findMax(filename);
	}
	cout << "Nodes: " << countOfNodes << endl;
	unsigned long *columns = new unsigned long[countOfNodes+1];
	fill_n(columns, countOfNodes+1, 0);
	
	cout << "Columns counting... ";
	countColumns(filename, columns);
	
	cout << "Preparing structure... ";
	unsigned long **rows = prepareStructure(columns, countOfNodes);

	cout << "Graph loading... ";
	loadGraph(filename, rows);
	
	cout << "Start of counting... ";
	triangles = countTriangles(rows, countOfNodes);
	
	cout << "Number of triangles: " << triangles << endl;

	return 0;
}
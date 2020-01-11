//Gerrymandring // FORMING WISCONSIN POLITICAL DISTRICT
#include <cmath>
#include <vector>
#include <cstdlib>//needed for rand()
#include <ctime>//needed for time()
#include <math.h>//needed for ceil()
#include <cstdlib>
//#include<stdio.h>
#include < ilconcert/iloenv.h>
#include <ilcplex/ilocplex.h>//needed for CPLEX
#include <fstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <array>

ILOSTLBEGIN
#define RC_EPS 1.0e-6

	static void readData1 (const char* filename, IloInt& noDistricts,IloNumArray& subPop);
static void readData2 (const char* filename, IloNumArray& democrats );
static void readData3 (const char* filename, IloNumArray& republicans );
static void readData4 (const char* filename, IloNumArray& area );
static void readData5 (const char* filename, IloNumArray& perimeter );
static void readData6 (const char* filename, IloNumArray& voter );
static void readData7 (const char* filename, IloNumArray& amount );
static void readData8 (const char* filename, IloNumArray2& bLength );
static void readData9 (const char* filename, IloNumArray2& adjacency );
static void readData10 (const char* filename, IloNumArray& black );
static void readData11 (const char* filename, IloNumArray& hispanic );
static void readData12 (const char* filename, IloNumArray& address );
static void readData13 (const char* filename, IloNumArray& senAddress );
static void readData14 (const char* filename, IloNumArray2& split );


// Definition of vectos for checking contiguity
static vector<int> traversed_nodes;
static vector<int> districts_to_check_contiguity;
static vector<int> subs_in_district;
int traverseSubs(int node, IloNumArray2 adjacency);
int listsizeHP;
int listsizeLP;
int listsizeHG;
bool detailed_output=false;
bool very_detailed_output=false;
bool initial_output=false;
bool edges_output=false;
bool neighbor_output=false;
bool contiguety_output=false;
bool evaluation_output=false;

bool sortinrev(const pair<int,int> &a,  const pair<int,int> &b) 
{ 
	return (a.first > b.first); 
} 

/// MAIN PROGRAM ///
int main(int argc, char **argv)
{
	unsigned long int startTime=time(NULL);//random is system time in sec.
	unsigned long int random= time(NULL);//fixed system time
	//unsigned long int random=  1572734411;//fixed system time
	//cout<<"Our random number seed = "<<random<<endl;
	srand(random);//uses random to set the seed for random # generation

	IloEnv env;

	unsigned long int initialtime=0;
	long int myTime0 = time(NULL);

	try
	{
		IloInt  i, j, k;
		IloInt      noDistricts;
		IloNum      Vote;
		IloNum      Pbar;
		IloNumArray subPop(env);
		IloNumArray democrats(env);
		IloNumArray2 bLength(env);
		IloNumArray amount(env); 
		IloNumArray area(env);
		IloNumArray perimeter(env);
		IloNumArray voter(env);
		IloNumArray republicans(env);
		IloNumArray2 adjacency(env);
		IloNumArray corners(env);
		IloNumArray black(env); 
		IloNumArray hispanic(env);
		IloNumArray address(env); 
		IloNumArray senAddress(env); 
		IloNumArray2 split(env);

		//Weights
		IloNum  w1, w2, w3, w4;
		w1 = 1;
		w2 = 0.05; 
		w3 = 0;
		w4 = 0; 

		// Simulated Anealing Method:///////////////////////////////////////////////
		//(1) The following parameters relate to the simulated annealling procedure.
		const int NUM_ITERATIONS = 100; 
		const double TEMP_FACTOR = 0.997; //0.9 //0.99 //0.999 //0.995
		const double START_TEMPERATURE = 0.01; //0.01; //1000 //400 //50 // 300 // 100 //200

		//(2) Use a simulated annealing neighborhood search procedure to improve the initial solution.
		double acceptanceProbability;
		double temperature = START_TEMPERATURE;
		double FinalTemp = 0.01; //0.1 // If needed
		long int steps = 0;
		long int iterations = 0;
		bool accept;    
		bool reject;
		bool given;
		int noOfGeneration=0;
		int noOfFeasibleSolution=0;
		long int bestIteration = 0;
		long int bestStep = 0;

		double populationDeviation=0.02; //districts' population deviation
		int noInitialGeneration=5; //no of initial maps
		long int algorithmRunTime=60; 
		// Vectors for calculating districts penalty 
		int n;
		vector < double >  distPop;
		vector < double >  distPerimeter;
		vector < double >  distDem;
		vector < double >  distVote;
		vector < double >  distArea;
		vector < double >  distFPop;
		vector < double >  distFPerimeter;
		vector < double >  distFDem;
		vector < double >  distFVote;			
		vector < double >  distFArea;		
		vector < double > Penalty;
		vector < double > PenaltyP;
		vector < double > PenaltyV;
		vector < double > PenaltyCom;
		vector < double > PenaltyR;
		vector < double > OriginalPenalty;
		vector < double > OriginalPenaltyP; 
		vector < double > OriginalPenaltyV;
		vector < double > OriginalPenaltyCom;
		vector < double > OriginalPenaltyR;
		vector < double > OriginalFPenalty;		      
		vector < double > OriginalFPenaltyP;		
		vector < double > OriginalFPenaltyV;		
		vector < double > OriginalFPenaltyCom;
		vector < double > OriginalFPenaltyR;

		vector < double > assignedMicroadjacency; 
		vector < int > dependentWards; 
		vector <vector <int >> accompanyingWards; // to store wards with only one neighbor
		vector <vector <int >> splitWards; // to store split wards
		vector <vector <int >> neighbors; // to store split wards
		vector < double > insideWards;
		vector < double > myNotContiguousWards;
		///////////////////////////////////////////////////////////////////////////
		// Variables for normalization
		double meanP=0;
		double meanV=0;
		double meanVE=0;
		double meanC=0;
		double meanR=0;
		double Sd1=0; 
		double Sd2=0; 
		double Sd3=0; 
		double SdP;
		double SdV;
		double SdVE;
		double SdC;
		double SdR; 
		///////////////////////////////////////////////////////////////////////////////				
		//reading data from files 
		if ( argc > 1 )
			readData1 (argv[1], noDistricts, subPop);
		else
			readData1("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\PopCor1.txt", noDistricts, subPop);

		if ( argc > 1 )
			readData2 (argv[1], democrats);
		else
			readData2("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\DemocratsN.txt", democrats);
		/*
		if ( argc > 1 )
		readData3 (argv[1], republicans);
		else
		readData3("C:\\Users\\royag\\Dropbox\\UWM\\Dissertation\\Gerrymandering\\Project 4\\WisconsinData\\Version1\\FinalReadableData\\Republicans.txt", republicans);
		*/		
		if ( argc > 1 )
			readData4 (argv[1], area);
		else
			readData4("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\Area.txt", area);

		if ( argc > 1 )
			readData5 (argv[1], perimeter);
		else
			readData5("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\Perimeter.txt", perimeter);

		if ( argc > 1 )
			readData6 (argv[1], voter);
		else
			readData6("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\VotersN.txt", voter);

		if ( argc > 1 )
			readData7 (argv[1], amount);
		else
			readData7("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\amount.txt", amount);

		if ( argc > 1 )
			readData8 (argv[1],bLength);
		else
			readData8("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\length.txt", bLength);

		if ( argc > 1 )
			readData9 (argv[1],adjacency);
		else
			readData9("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\adjacent.txt", adjacency);

		if ( argc > 1 )
			readData10 (argv[1],black);
		else
			readData10("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\BlackN.txt", black);

		if ( argc > 1 )
			readData11 (argv[1],hispanic);
		else
			readData11("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\HispanicN.txt", hispanic);

		if ( argc > 1 )
			readData12 (argv[1],address);
		else
			readData12("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\RepAddress.txt", address);


		if ( argc > 1 )
			readData13 (argv[1],senAddress);
		else
			readData13("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\SenAddress.txt", senAddress);


		if ( argc > 1 )
			readData14 (argv[1],split);
		else
			readData14("C:\\Users\\royag\\Documents\\MyDissFiles\\WisconsinData_Processing\\BeChecked\\2018data\\Splt9.txt", split);


		// find the numbers of micro_districts 
		IloInt numSubs = amount.getSize()-1;
		cout << "numSubs=" << numSubs << endl; 
		//cout << "democrats=" << democrats << endl;

	/*	////////////////////Data Correction for splt6, 7, 10////////////////////

		adjacency [4761][4664]=0;
		adjacency [4761][4665]=0;
		adjacency [4761][4688]=0;
		adjacency [4761][4735]=0;
		adjacency [4761][4771]=0;
		adjacency [4761][4774]=0;

		adjacency [5818][5883]=0;
		adjacency [5818][4797]=0;

		adjacency [4664][4761]=0;
		adjacency [4665][4761]=0;
		adjacency [4688][4761]=0;
		adjacency [4735][4761]=0;
		adjacency [4771][4761]=0;
		adjacency [4774][4761]=0;

		adjacency [5883][5818]=0;
		adjacency [4797][5818]=0;

		bLength [4761][4664]=0;
		bLength [4761][4665]=0;
		bLength [4761][4688]=0;
		bLength [4761][4735]=0;
		bLength [4761][4771]=0;
		bLength [4761][4774]=0;

		bLength [5818][5883]=0;
		bLength [5818][4797]=0;

		bLength [4664][4761]=0;
		bLength [4665][4761]=0;
		bLength [4688][4761]=0;
		bLength [4735][4761]=0;
		bLength [4771][4761]=0;
		bLength [4774][4761]=0;

		bLength [5883][5818]=0;
		bLength [4797][5818]=0;

		//*////////////////////////////////////
		//Correction of neighbors
		adjacency [3368][4311]=0;
		adjacency [4311][3368]=0;
		bLength [3368][4311]=0;
		bLength [4311][3368]=0;

		///////////////////////////////////////

		// Find Average Population, Average number of microdistrict in each district and vote percentage for democrates in state 
		double GapD;  // Democrats gap
		double GapR; // Republicans gap
		double sumDGap;
		double sumRGap;
		int Dem; // # of dem seats 
		int Rep; // # of rep seats 
		double VD=0; // Total Democrats
		double VR; // Total Republicans
		double TVoter=0; // Total voter 
		double Pw; // Average population for each ward(micro district)
		double Pm=0; // micro_district population 
		for(int m=0; m< numSubs ; m++)
			Pm =Pm+subPop[m]; 

		Pbar= Pm / noDistricts; // Average Population (Per district)
		//n = numSubs / noDistricts;
		Pw=Pm/numSubs; // Average population per micro-district(ward) 

		for(int m=0; m< numSubs ; m++){
			//republicans[m]=voter[m]-democrats[m];
			VD=VD+democrats[m]; // Total Democrats
			TVoter=TVoter+voter[m]; //Total voters
		}

		VR=TVoter-VD; // Total Republicans
		Vote = VD / TVoter;  // percentage of Democrats in state 
		double AvgNumDemocratsPerDistrict= VD/noDistricts;  // Average Democrats per district

		cout << "percentage of Democrats in state =" << Vote << endl;
		cout << "AveragePopPerDistrict=" << Pbar << endl;
		cout << "AvgNumDemocratsPerDistrict=" << AvgNumDemocratsPerDistrict << endl;
		cout << "Pm=" << Pm << endl; 
		cout << "Total voters in state" << TVoter << endl; 

		//Create an output file for displaying the final results.
		ofstream myOutputFile;
		char excelOutputFileName[] = {'f','i','l','e','0','0','0','.','t','x','t','\0'};
		for (i=0;i<=10;i++)
			cout << excelOutputFileName[i];
		cout << endl;
		myOutputFile.open(excelOutputFileName);
		if (myOutputFile.fail())
		{
			cerr << "File called " << excelOutputFileName << " could not be opened." << endl;
			return 1;
		}
		/*
		ofstream myOutputFile1;
		char excelOutputFile1Name[] = {'f','i','l','e','1','1','1','.','t','x','t','\0'};
		for (i=0;i<=10;i++)
			cout << excelOutputFile1Name[i];
		cout << endl;
		myOutputFile1.open(excelOutputFile1Name);
		if (myOutputFile1.fail())
		{
			cerr << "File called " << excelOutputFile1Name << " could not be opened." << endl;
			return 1;
		}
		ofstream myOutputFile2;
		char excelOutputFile2Name[] = {'f','i','l','e','2','2','2','.','t','x','t','\0'};
		for (i=0;i<=10;i++)
			cout << excelOutputFile2Name[i];
		cout << endl;
		myOutputFile2.open(excelOutputFile2Name);
		if (myOutputFile2.fail())
		{
			cerr << "File called " << excelOutputFile2Name << " could not be opened." << endl;
			return 1;
		}
		ofstream myOutputFile3;
		char excelOutputFile3Name[] = {'f','i','l','e','3','3','3','.','t','x','t','\0'};
		for (i=0;i<=10;i++)
			cout << excelOutputFile3Name[i];
		cout << endl;
		myOutputFile3.open(excelOutputFile3Name);
		if (myOutputFile3.fail())
		{
			cerr << "File called " << excelOutputFile3Name << " could not be opened." << endl;
			return 1;
		}
		*/
		myOutputFile <<"Our random number seed = "<<random<<endl;
		//---------------------------------------------------------------------------------------------------------------------

		//
		// Generate the initial feasible solution
		// first index refers to rows and second index refers to columns
		int initial [6977][99];
		int binitial [6977][99];
		double firstPenalty;  // Weighted Normalized total penalty
		double firstPenaltyP; // Weighted Normalized total Pop penalty
		double firstPenaltyV; // Weighted Normalized total Political fairness penalty
		double firstPenaltyCom; // Weighted Normalized total compactness penalty
		double firstPenaltyR; // Weighted Normalized total Residential penalty

		double OriginalFirstPenalty;  // unWeighted Normalized total penalty
		double OriginalFirstPenaltyP; //unWeighted Normalized total Pop penalty
		double OriginalFirstPenaltyV; // unWeighted Normalized total Political fairness penalty
		double OriginalFirstPenaltyCom; // unWeighted Normalized total compactness penalty
		double OriginalFirstPenaltyR; // unWeighted Normalized total Residential penalty

		double beforeNormalFirstPenalty;//Unweighted not Normalized total penalty
		double beforeNormalFirstPop;  //unWeighted not Normalized total Pop penalty
		double beforeNormalFirstVote;  // unWeighted not Normalized total Political fairness penalty
		double beforeNormalFirstCom; // unWeighted not Normalized total compactness penalty
		double beforeNormalFirstRes; // unWeighted not Normalized total Residential penalty
		//double beforeNormalFirstVote1; 

		//2 dim vectors for generating initial districts
		static vector<int> district_seeds;        
		vector<vector<int>> district_nodes;
		//2 dim vectors for district containts and edges
		vector<vector<int>> corners_nodes;
		vector<vector<int>> district_wards;
		vector<vector<int>> Tempdistrict_wards;
		vector<vector<int>> best_district_wards;

		//2 dime vectors for storing the district of each words; 
		vector<vector<int>> ward_districts;
		vector<vector<int>> Tempward_districts;

		//2 dime vectors for neighbors of final best district
		vector<vector<int>> neighbor_districts;


		//vectors for storing districts' population, democrats' votes , Area and perimeter
		vector<double> Popvect;
		vector<double> Demvect;
		vector<double> Repvect;
		vector<double> Areavect;
		vector<double> Perivect;
		vector<double> RGapvect;
		vector<double> DGapvect;

		//Temporary vectors for storing districts' population, dem votes , Area and perimeter
		vector<double> TPopvect;
		vector<double> TDemvect;
		vector<double> TRepvect;
		vector<double> TAreavect;
		vector<double> TPerivect;
		vector<double> TRGapvect;
		vector<double> TDGapvect;

		vector<double> Address;
		for (int i=0; i< noDistricts ; i++){
			Address.push_back(address[i]);
		}
	
		vector<double> SenAddress;
		for (int i=0; i< 33 ; i++){
			SenAddress.push_back(senAddress[i]);
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////
		//Find the wards with only one neighbor////////
		///////////////////////////////////////////////
		int W;	
		int cc;
		accompanyingWards.resize(numSubs);
		for(i=0; i<numSubs; i++){
			W=0;
			for(j=0; j<numSubs; j++){
				W=W+adjacency[i][j];
				if(adjacency[i][j]==1){
					cc=j;
				}
			}
			if (W==1){
				accompanyingWards[cc].push_back(i); 
				//cout << "cc=" << cc << endl; 
			}				 
		}

		if (detailed_output==false){
			int e=0; 
			for(i=0; i<numSubs; i++){
				e=e+accompanyingWards[i].size();
			}
			cout << "accompanyingWards.size()=" << e << endl; 
		}

		///////////////////////////////////////////////////////
		//Find the wards which area inside another wards (one neighbor)////////
		///////////////////////////////////////////////////////

		for (int i=0; i <accompanyingWards.size(); i++){
			if (accompanyingWards[i].size() > 0){
				if(ceil(perimeter[accompanyingWards[i][0]])==ceil(bLength[i][accompanyingWards[i][0]])){				
					insideWards.push_back(accompanyingWards[i][0]);
				}
				//else{					
					//myOutputFile3 << "to be checked" << accompanyingWards[i][0] << endl; 
				//}
			}
		}

		if (detailed_output==false){
			cout << "insideWards.size()=" << insideWards.size() << endl; 
		}
		////////////////////////////////////////////////////////////
		// store split wards (exclude which are already considered as inside wards and single number)
		/////////////////////////////////////////////////////////////
		vector<vector<int>> MySplit;	
		splitWards.resize(numSubs);
		MySplit.resize(numSubs);

		for(i=0; i<numSubs; i++){
			if(std::find(insideWards.begin(), insideWards.end(),i) == insideWards.end()){
				for(j=0; j<numSubs; j++){
					if(std::find(insideWards.begin(), insideWards.end(), j) == insideWards.end()){
						if(split[i][j]==1){						
							MySplit[i].push_back(j); 
						}
					}
					else{
						continue;
					}
				}
			}
			else{
				continue;
			}
		}


		///////////////////////////////////////////
		/*
		for(i=0; i<numSubs; i++)
			for(j=0; j<MySplit[i].size(); j++){ 
				if (i==4634){
					cout <<  "4434 and " << j << "and" << MySplit[i][j]<< endl;
				}
			}
*/
			/////////////////Test//////////////////////////////////

			int e=0;	
			for(i=0; i<numSubs; i++)
				for(j=0; j<MySplit[i].size(); j++){
					for (k=0; k< insideWards.size() ; k++) {

						if(std::find(splitWards[i].begin(), splitWards[i].end(),insideWards[k]) == splitWards[i].end()){
							continue;
						}
						else{
							cout << insideWards[k]<< endl;
							e=e+1;
						}
					}															 
				}

				cout << "myinside=" << e << endl; 


				//////////////////////////////////////////////////////////
				///////////////////////////////////////////////////////
				////Store a chain of district which are moving together
				/////////////////////////////////////////////////////////
				for (int i=0; i<numSubs ; i++){

					if(MySplit[i].size()>0){
						int cs=0; 
						int cf=MySplit[i].size(); 
						for (int j=0; j < MySplit[i].size(); j++) {
							splitWards[i].push_back( MySplit[i][j] );
						}


						for (int k=cs; k<cf ;k++) {
							//cout << giving_node_split[i] << endl; 
							//cout << "splitWards[giving_node_split[i]].size()"<< splitWards[giving_node_split[i]].size() << endl; 
							int h = splitWards[i][k];
							if (MySplit[h].size() > 0) {
								for (int jj=0 ; jj < MySplit[h].size(); jj++){	

									if(std::find(splitWards[i].begin(), splitWards[i].end(), MySplit[h][jj]) == splitWards[i].end() && MySplit[h][jj]!=i ){

										splitWards[i].push_back(MySplit[h][jj]);	 
									}
								}	

							} else{
								continue;
							}
							cs=cf;
							cf=splitWards[i].size();

						}

					}
				}

				//*
				//for(i=0; i<numSubs; i++){
				for(j=0; j<splitWards[1234].size(); j++){

					cout  <<"split1234" <<j <<  splitWards[835][j]<< endl; 

				}															 
				//	}


				//*/

				e=0; 
				for(i=0; i<numSubs; i++){

					e=e+splitWards[i].size();
				}
				cout << "splitWards.size()=" << e << endl; 



				////////////////////Add accompanying wards with one neighbor to split chain which is not inside the other wards///////////

				for (int i=0; i <accompanyingWards.size(); i++){
					if (accompanyingWards[i].size() > 0){
						if(ceil(perimeter[accompanyingWards[i][0]])!=ceil(bLength[i][accompanyingWards[i][0]])){
							accompanyingWards[i].erase(std::remove(begin(accompanyingWards[i]), end(accompanyingWards[i]), accompanyingWards[i][0]), end(accompanyingWards[i]));					
							if(std::find(splitWards[i].begin(), splitWards[i].end(),accompanyingWards[i][0]) == splitWards[i].end()){
								splitWards[accompanyingWards[i][0]].push_back(i);
							}
							if(std::find(splitWards[accompanyingWards[i][0]].begin(), splitWards[accompanyingWards[i][0]].end(),i) == splitWards[accompanyingWards[i][0]].end()){
								splitWards[accompanyingWards[i][0]].push_back(i);
							}
						}
					}
				}

				//////////////////////////////////////////////////////////////////

				/*
				for(i=0; i<numSubs; i++){
				for(j=0; j<splitWards[i].size(); j++){

				cout <<i <<"split" <<j <<  splitWards[i][j]<< endl; 

				}															 
				}
				*/
				e=0; 
				for(i=0; i<numSubs; i++){

					e=e+splitWards[i].size();
				}
				cout << "splitWards.size()=" << e << endl; 




				/////////////////////////////////////////////////
				//  generate 100 initial **********************//
				////////////////////////////////////////////////
				int iniGenNumber = 0; // Number of initial districts 
				double bestbefnormpen = 999999999999999999; 

				// Vectors for mean and standard deviations(population, political fairness and compactness) for 100 initials
				vector<double> listOfmeanP , listOfmeanV, listOfmeanVE , listOfmeanC , listOfmeanR , listOfSdP , listOfSdV, listOfSdC, listOfSdR ,listOfInia ,listOfInib ;

				do {
					firstPenalty=0;  // Weighted Normalized total penalty
					firstPenaltyP=0; // Weighted Normalized total Pop penalty
					firstPenaltyV=0; // Weighted Normalized total Political fairness penalty
					firstPenaltyCom=0; // Weighted Normalized total compactness penalty
					firstPenaltyR=0;

					OriginalFirstPenalty=0;  // unWeighted Normalized total penalty
					OriginalFirstPenaltyP=0; //unWeighted Normalized total Pop penalty
					OriginalFirstPenaltyV=0; // unWeighted Normalized total Political fairness penalty
					OriginalFirstPenaltyCom=0; // unWeighted Normalized total compactness penalty
					OriginalFirstPenaltyR=0; // unWeighted Normalized total compactness penalty

					beforeNormalFirstPenalty=0;//Unweighted not Normalized total penalty
					beforeNormalFirstPop=0;  //unWeighted Normalized total Pop penalty
					beforeNormalFirstVote=0;  // unWeighted Normalized total Political fairness penalty
					beforeNormalFirstCom=0; // unWeighted Normalized total compactness penalty
					beforeNormalFirstRes=0;
					//double beforeNormalFirstVote1; 

					iniGenNumber= iniGenNumber+1;

					// put the elements of initial matrix equal zero
					for (i=0; i < numSubs; i++)
						for(j=0; j<noDistricts; j++)
							initial[i][j]=0; 


					// -- START: dynamic initialization of districts based on traversing
					for (int ii = 0; ii < noDistricts; ii++) {
						if (initial_output==true){
							cout << "sub no " << ii;
							//myOutputFile1 << "sub no " << ii;
						}
						vector<int> tmp_vec;
						int tmp_seed;
						do {
							tmp_seed = rand() % numSubs;
						} while (std::find(district_seeds.begin(), district_seeds.end(), tmp_seed) != district_seeds.end() || splitWards[tmp_seed].size() > 0 || accompanyingWards[tmp_seed].size()>0);

						district_seeds.push_back(tmp_seed); 
						initial[tmp_seed][ii] = 1; 
						tmp_vec.push_back(tmp_seed); // create a single-member vector using the new seed 
						district_nodes.push_back(tmp_vec); // put the temp vector in the 2D vector
						vector<int>().swap(tmp_vec); // free the temp vector to avoid memory hog
					}

					if (initial_output==true){
						//myOutputFile1 << "\n\n District_seeds: "; 
						cout << "\n\n District_seeds: "; 
						for (int i = 0; i < district_nodes.size(); i++)  {               
							for (int j = 0; j < district_nodes[i].size(); j++) {     
								//myOutputFile1 << district_nodes[i][j] << ", ";
								cout << district_nodes[i][j] << ", ";
							}
						}
					}

					bool microdists_left;
					do {
						microdists_left = false;
						// district_seeds_temp.clear();
						for (int i = 0; i < district_nodes.size(); i++)  {               
							if ( district_nodes[i].size() == 0 ) {
								if (initial_output==true){
									//myOutputFile1 << "\nDist. " << i << " has run out of bLength to grow."; 
									cout << "\nDist. " << i << " has run out of bLength to grow."; 
								}
								continue; // no free neighbors left to grab, go to next seed
							} else {
								microdists_left = true;
							}
							vector<int> temp_neighbor_candidate;
							for (int k = 0; k < numSubs; k++) {
								// found a neighbor sub?                    
								if (adjacency[district_nodes[i].back()][k] == 1) {
									if (initial_output==true){
										//myOutputFile1 << "\n  Neighbor for node " << district_nodes[i].back() << ": node " << k;
										cout << "\n  Neighbor for node " << district_nodes[i].back() << ": node " << k;
									}
									// now is it unclaimed? i is index, *i is sub number
									bool already_taken = false;
									for (int m = 0; m < noDistricts; m++) {
										if (initial[k][m] == 1) {  
											// check to see if K is already taken by one of the districts
											if (initial_output==true){
												//myOutputFile1 << " - already belongs to dist. " << m; 
												cout << " - already belongs to dist. " << m; 
											}
											already_taken = true;
											break;
										}
									}
									if (already_taken == true) { 
										continue; 
									} else {
										// push back all neighbors of last element of the vector
										temp_neighbor_candidate.push_back(k);        
									}
								}
							}
							if (initial_output==true){
								//myOutputFile1 << "temp_neighbor_candidate.size()=" << temp_neighbor_candidate.size() << endl;
								cout << "temp_neighbor_candidate.size()=" << temp_neighbor_candidate.size() << endl;
							}
							// check to see if the last element of vector did end up having at least one neighbor (above code)?
							if ( temp_neighbor_candidate.size() > 0 ) {
								// if yes, now pick one from the list of available neighbors
								int chosen_node = temp_neighbor_candidate[ rand() % temp_neighbor_candidate.size() ];
								// add the neighbor to the district
								initial[chosen_node][i] = 1;    
								//add the neighbor to single vector that we are using for traverse
								district_nodes[i].push_back(chosen_node); 
								// also push back any glued wards
								if(splitWards[chosen_node].size() > 0){
									for (int j =0; j< splitWards[chosen_node].size(); ++j) {
										//myOutputFile1 << j <<"----" <<splitWards[chosen_node][j] << endl; 
										initial[splitWards[chosen_node][j]][i] = 1; 
										district_nodes[i].push_back(splitWards[chosen_node][j]);
										//myOutputFile1 << "\nnode " << splitWards[chosen_node][j] << " now assigned to dist. " << i << " GLUED"; 
										//myOutputFile1 << " --  initial[" << splitWards[chosen_node][j] << "][" << i << "] = 1";		

									}
								}


								// also push back any inside wards
								if(accompanyingWards[chosen_node].size() > 0){
									for (int j =0; j< accompanyingWards[chosen_node].size(); ++j) {
										//myOutputFile1 << j <<"----" <<accompanyingWards[chosen_node][j] << endl; 
										initial[accompanyingWards[chosen_node][j]][i] = 1; 
										district_nodes[i].push_back(accompanyingWards[chosen_node][j]);
										//myOutputFile1 << "\nnode " << accompanyingWards[chosen_node][j] << " now assigned to dist. " << i << " GLUED"; 
										//myOutputFile1 << " --  initial[" << accompanyingWards[chosen_node][j] << "][" << i << "] = 1";		

									}
								}



								if (initial_output==true){
									//myOutputFile1 << "\nnode " << chosen_node << " now assigned to dist. " << i; 
									//myOutputFile1 << " --  initial[" << chosen_node << "][" << i << "] = 1";
									cout << "\nnode " << chosen_node << " now assigned to dist. " << i; 
									cout << " --  initial[" << chosen_node << "][" << i << "] = 1";
								}
							} else {
								if (initial_output==true){
									//myOutputFile1 << "\nnode " << district_nodes[i].back() << " is a dead end! removing from traverse list. " << i;
									cout << "\nnode " << district_nodes[i].back() << " is a dead end! removing from traverse list. " << i;
								}
								// if couldn't find a neighbor for last element of vector, delet it
								district_nodes[i].pop_back();
							}
						}
						//cout << " \n=================================================== ";
					} while (microdists_left == true);

					// memory cleanup?
					for (int ii = 0; ii < noDistricts; ii++) {
						district_nodes[ii].clear();
						vector<int>().swap(district_nodes[ii]);
					}
					vector<vector<int>>().swap(district_nodes);
					vector<int>().swap(district_seeds);
					// -- END: dynamic initialization of districts based on traversing

					///////////Test/////////////////////////////////////
					if (initial_output==true){
						int sum; 
						for (int i=0; i< numSubs; i++) {
							sum=0;
							for (int j=0; j<noDistricts ; j++)
							{
								sum= sum + initial [i][j] ;
							}
							if (sum < 1) {
								listOfInia.push_back(i);
							}
						}
						//myOutputFile1 << "listOfInia.size "<<listOfInia.size() << ":" ;
						//myOutputFile1 << "unassigned nodes" << endl; 
						cout << "listOfInia.size "<<listOfInia.size() << ":" ;
						cout << "unassigned nodes" << endl; 
						for (auto ii=listOfInia.begin() ; ii< listOfInia.end() ; ii++){
							//myOutputFile1 << *ii << "," ; 
						}

						//myOutputFile1 << "neighbors=" << endl; 
						cout << "neighbors=" << endl; 
						for (auto ii=listOfInia.begin() ; ii< listOfInia.end() ; ii++){
							for (int j=0; j< numSubs; j++) { 
								if (adjacency[*ii][j]==1){
									listOfInib.push_back(j);
									//myOutputFile1 << "neighbor["<<*ii << " ]:" << j << "," ;
									cout << "neighbor["<<*ii << " ]:" << j << "," ;
								}
							}
							listOfInib.clear();
						}

						////////////////////////////////////
						// Print Current Districs
						/*
						for (i=0;i<numSubs;i++)
						{
						for (j=0;j<noDistricts;j++)
						{
						myOutputFile << initial[i][j] << ",";
						}
						myOutputFile << endl;
						}
						//*/
						/////////////Test/////////////////
						int cn =0; 
						int Tcn =0;
						for (i=0;i<noDistricts;i++)
						{
							cn=0; 
							for (j=0;j<numSubs;j++){
								if ( initial [j][i] ==1) {
									cn=cn+1;
								}
							}
							//myOutputFile1 << " Count col ["<< i << "]=" << cn  << endl ;
							cout << " Count col ["<< i << "]=" << cn  << endl ;
							Tcn = Tcn+cn;
						}
						//myOutputFile1 << "Tcn "<< Tcn << endl; 
						cout << "Tcn "<< Tcn << endl; 
					}
					////////////////////////////////////////////////////////////////////////////////////////////
					// find the first Penalty for each generated district
					distPop.resize(numSubs+1);
					distVote.resize(numSubs+1);
					distDem.resize(numSubs+1);
					distPerimeter.resize(numSubs+1); 
					distArea.resize(numSubs+1);
					//distDGap.resize(numSubs+1);
					//distRGap.resize(numSubs+1);

					Rep=0; // # of Rep seats
					Dem=0; // # of Dem seats			
					double P;  // district population 
					double D;  // democrats population  per district
					double R;  // Republicans population  per district			 
					double Temp; // for perimeter
					double sum; // for area
					double G;   //50 % +1  of district population
					GapD=0;  // Democrats gap
					GapR=0; //Republicans gap;
					sumDGap=0;
					sumRGap=0;
					int Res=0;
					int r;
					for (int i = 0; i < noDistricts; i++)
					{

						r=0;
						// computing distPop[i]
						P = 0;  // Population of each district
						D = 0;  // Democrats of each district
						R = 0;  // Republicans of each district
						sum=0; // for area
						Temp=0; // for perimeter
						for (int j = 0; j < numSubs ; j++ )
						{
							int c=0;
							// Penalty for population
							P += (subPop[j]*initial[j][i]); 
							//Penalty for deviation of democrate vote
							D += (democrats[j]*initial[j][i]);
							R += ((voter[j]-democrats[j])*initial[j][i]);					
							//// Penalty for compactness 
							//finding the perimeter of each district					
							if (initial[j][i]>0.99) 
							{
								for (int k=0; k<numSubs; k++)
								{

									if ( initial [k][i]==1 && adjacency[j][k]> 0.99) 
									{                   
										c += bLength[j][k];     
									}
								}
							}
							// finding perimeter of each district


							Temp = Temp + perimeter[j]-c; 
							// finding area of each district
							sum = sum + area[j] * initial[j][i];

							// Representatives' residency  


							if (initial[j][i]>0.99) 
							{
								for (auto ii=Address.begin() ; ii < Address.end() ; ii++){
									if (j==*ii){
										r=r+1;
									}
								}
							}
						}

						// Efficiency Gap (based on second definition of political fairness)
						/*
						G = ceil (0.50* (R+D));
						if (D >= G) {
						GapR = R ; 
						GapD = D - G;
						}
						else { 
						GapD = D ; 
						GapR = R - G;
						}
						//*/

						// Parties' Seats (based on third definition of political fairness)
						//*
						if (R > D){
							Rep=Rep+1;
						}
						else{
							Dem=Dem+1;  
						}
						//*/

						//cout << " GapD , GapR" << GapD << ","<< GapR<< endl;        

						//(1)Pop penalty for each district
						distPop[i] = abs(P-Pbar);
						//distPop[i] = abs(pow((P-Pbar),2));
						//distPop[i] = abs(pow((P-Pbar),3));

						//(2)Political fairness penalty for each district
						// based on first definition of political fairness
						//distDem[i] = abs(D-AvgNumDemocratsPerDistrict);
						// based on second definition of political fairness(Efficiency gap)
						// sumDGap += GapD; //Total Democrats' Gap (state wide)
						// sumRGap += GapR; //Total Republicans' Gap (state wide)

						//(3)Lack of compactness(Perimeter and Area)

						distPerimeter[i]=Temp;    
						distArea[i]=sum;

						//(4)

						if(r>0){
							Res=Res+(r-1);
						}
					}

					if (initial_output==true){
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distPop" << i << "=" << distPop[i]<<endl; 
						cout << "distPop" << i << "=" << distPop[i]<<endl; 
						//for (i = 0; i < noDistricts ; i++)
						//	myOutputFile1 << "distDem" << i << "=" << distDem[i]<<endl;	
						//	cout << "distDem" << i << "=" << distDem[i]<<endl;	
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distPerimeter" << i << "=" << distPerimeter[i]<<endl;
						cout << "distPerimeter" << i << "=" << distPerimeter[i]<<endl;
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distArea" << i << "=" << distArea[i]<<endl;
						cout << "distArea" << i << "=" << distArea[i]<<endl;
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "Compactness" << i << "=" << pow(distPerimeter[i],2)/distArea[i]<<endl;
						cout << "Compactness" << i << "=" << pow(distPerimeter[i],2)/distArea[i]<<endl;
					}
					// find mean and standard deviation for political fairness penalty and population 
					meanP=0; 
					meanV=0;
					meanC=0; 
					meanR=0;

					for (int i = 0; i < noDistricts; i++)
					{
						meanP += (distPop[i]);
						//meanV += (distDem[i]);
						meanC += pow(distPerimeter[i],2)/distArea[i];  //based on the second approach
					}
					meanP /= noDistricts;
					//meanV /= noDistricts;
					meanC /= noDistricts;
					meanR =Res; 

					//For Efficiency Gap (Second definition of political fairness)
					//meanV = abs((sumDGap)/VD-(sumRGap)/VR);  // Can be changed
					//based on Third definition of political fairness(# of parties seats)
					meanV = abs(Rep - ceil ((1-Vote)*noDistricts));  // Or meanV = abs(Dem - ceil ((Vote)*noDistricts));


					listOfmeanP.push_back(meanP); 
					listOfmeanV.push_back(meanV);
					listOfmeanC.push_back(meanC); 
					listOfmeanR.push_back(meanR);

					for( int i = 0; i < noDistricts; i++ )
					{
						Sd1 += pow((distPop[i] - meanP), 2);
						//Sd2 += pow((distDem[i] - meanV), 2);            
						// Compactness equal to perimeter square over area
						Sd3 += pow(((pow((distPerimeter[i]),2)/distArea[i]) - meanC),2);

					}


					Sd1 /= noDistricts;
					//Sd2 /= noDistricts;
					Sd3 /= noDistricts;

					SdP = sqrt(Sd1);
					//SdV = sqrt(Sd2);
					SdC = sqrt(Sd3);


					listOfSdP.push_back(SdP); 
					//listOfSdV.push_back(SdV); 
					listOfSdC.push_back(SdC); 

				} while (iniGenNumber < noInitialGeneration);  // for generating 100 initials 



				////////////////////////////////////////////////////////////////////////////////////////////////
				// average mean and standard deviation of 100 initials
				double TempMeanP=0; 
				for (auto ii=listOfmeanP.begin() ; ii< listOfmeanP.end() ; ii++){
					TempMeanP += *ii; 
				}
				meanP= TempMeanP/ listOfmeanP.size() ; 
				//*
				double TempMeanV=0; 
				for (auto ii=listOfmeanV.begin() ; ii< listOfmeanV.end() ; ii++){
					TempMeanV += *ii; 
				}
				meanV= TempMeanV/ listOfmeanV.size() ; 
				//*/
				double TempMeanC=0; 
				for (auto ii=listOfmeanC.begin() ; ii< listOfmeanC.end() ; ii++){
					TempMeanC += *ii; 
					//cout << "TempMeanC=" << TempMeanC << endl; 
				}
				meanC= TempMeanC / listOfmeanC.size() ; 

				double TempMeanR=0; 
				for (auto ii=listOfmeanR.begin() ; ii< listOfmeanR.end() ; ii++){
					TempMeanR += *ii; 
					//cout << "TempMeanC=" << TempMeanC << endl; 
				}
				meanR= TempMeanR / listOfmeanR.size() ;



				double TempSdP=0; 
				for (auto ii=listOfSdP.begin() ; ii< listOfSdP.end() ; ii++){
					TempSdP += *ii; 
				}
				SdP= TempSdP/ listOfSdP.size() ; 
				/*
				double TempSdV=0; 
				for (auto ii=listOfSdV.begin() ; ii< listOfSdV.end() ; ii++){
				TempSdV += *ii; 
				}
				SdV= TempSdV/ listOfSdV.size() ;
				//*/
				//*
				double TempSdV=0; 
				for (auto ii=listOfmeanV.begin() ; ii< listOfmeanV.end() ; ii++){
					TempSdV +=pow((listOfmeanV[*ii] - meanV), 2) ; 
				}
				SdV =TempSdV/listOfmeanV.size(); 
				//*/

				double TempSdC=0; 
				for (auto ii=listOfSdC.begin() ; ii< listOfSdC.end() ; ii++){
					TempSdC += *ii; 
				}
				SdC= TempSdC/ listOfSdC.size() ; 

				//*
				double TempSdR=0; 
				for (auto ii=listOfmeanR.begin() ; ii< listOfmeanR.end() ; ii++){
					TempSdR +=pow((listOfmeanR[*ii] - meanR), 2) ; 
				}
				SdR =TempSdR/listOfmeanR.size();
				//*/

				cout << "meanP , meanV, meanC = " <<meanP << "," << meanV << "," << meanC << meanR << endl;
				cout << "SdP , SdV, SdC , SdR = " << SdP << "," << SdV << "," << SdC << SdR << endl;

				if (initial_output==true){
					//myOutputFile1 << "meanP , meanV, meanC = " <<meanP << "," << meanV << "," << meanC << endl;
					cout << "meanP , meanV, meanC = " <<meanP << "," << meanV << "," << meanC << endl;

					//myOutputFile1 << "SdP , SdV, SdC = " << SdP << "," << SdV << "," << SdC << SdR << endl;
					cout << "SdP , SdV, SdC = " << SdP << "," << SdV << "," << SdC << SdR << endl;			
				}

				/////////////////////////////////////////////////////////////////////////////////////////////////////////

				iniGenNumber=0; 
				do {
					//cout << "bestbefnormpe=" << bestbefnormpen << endl; 
					firstPenalty=0;  // Weighted Normalized total penalty
					firstPenaltyP=0; // Weighted Normalized total Pop penalty
					firstPenaltyV=0; // Weighted Normalized total Political fairness penalty
					firstPenaltyCom=0; // Weighted Normalized total compactness penalty
					firstPenaltyR=0;

					OriginalFirstPenalty=0;  // unWeighted Normalized total penalty
					OriginalFirstPenaltyP=0; //unWeighted Normalized total Pop penalty
					OriginalFirstPenaltyV=0; // unWeighted Normalized total Political fairness penalty
					OriginalFirstPenaltyCom=0; // unWeighted Normalized total compactness penalty
					OriginalFirstPenaltyR=0;

					beforeNormalFirstPenalty=0;//Unweighted not Normalized total penalty
					beforeNormalFirstPop=0;  //unWeighted Normalized total Pop penalty
					beforeNormalFirstVote=0;  // unWeighted Normalized total Political fairness penalty
					beforeNormalFirstCom=0; // unWeighted Normalized total compactness penalty
					beforeNormalFirstRes=0;
					//double beforeNormalFirstVote1; 

					iniGenNumber= iniGenNumber+1;

					// put the elements of initial matrix equal zero
					for (i=0; i < numSubs; i++)
						for(j=0; j<noDistricts; j++)				
							initial[i][j]=0; 

					// -- START: dynamic initialization of districts based on traversing
					for (int ii = 0; ii < noDistricts; ii++) {
						if (initial_output==true){
							cout << "sub no " << ii;
							//myOutputFile1 << "sub no " << ii;
						}
						vector<int> tmp_vec;
						int tmp_seed;
						do {
							tmp_seed = rand() % numSubs;
						} while (std::find(district_seeds.begin(), district_seeds.end(), tmp_seed) != district_seeds.end() || splitWards[tmp_seed].size() > 0 || accompanyingWards[tmp_seed].size()>0);

						district_seeds.push_back(tmp_seed); 
						initial[tmp_seed][ii] = 1; 
						tmp_vec.push_back(tmp_seed); // create a single-member vector using the new seed 
						district_nodes.push_back(tmp_vec); // put the temp vector in the 2D vector
						vector<int>().swap(tmp_vec); // free the temp vector to avoid memory hog
					}

					if (initial_output==true){
						//myOutputFile1 << "\n\n District_seeds: "; 
						cout << "\n\n District_seeds: "; 
						for (int i = 0; i < district_nodes.size(); i++)  {               
							for (int j = 0; j < district_nodes[i].size(); j++) {     
								//myOutputFile1 << district_nodes[i][j] << ", ";
								cout << district_nodes[i][j] << ", ";
							}
						}
					}

					bool microdists_left;
					do {
						microdists_left = false;
						// district_seeds_temp.clear();
						for (int i = 0; i < district_nodes.size(); i++)  {               
							if ( district_nodes[i].size() == 0 ) {
								if (initial_output==true){
									//myOutputFile1 << "\nDist. " << i << " has run out of bLength to grow."; 
									//cout << "\nDist. " << i << " has run out of bLength to grow."; 
								}
								continue; // no free neighbors left to grab, go to next seed
							} else {
								microdists_left = true;
							}
							vector<int> temp_neighbor_candidate;
							for (int k = 0; k < numSubs; k++) {
								// found a neighbor sub?                    
								if (adjacency[district_nodes[i].back()][k] == 1) {
									if (initial_output==true){
										//myOutputFile1 << "\n  Neighbor for node " << district_nodes[i].back() << ": node " << k;
										//cout << "\n  Neighbor for node " << district_nodes[i].back() << ": node " << k;
									}
									// now is it unclaimed? i is index, *i is sub number
									bool already_taken = false;
									for (int m = 0; m < noDistricts; m++) {
										if (initial[k][m] == 1) {  
											// check to see if the node is already taken by one of the districts
											if (initial_output==true){
												//myOutputFile1 << " - already belongs to dist. " << m; 
												//cout << " - already belongs to dist. " << m; 
											}
											already_taken = true;
											break;
										}
									}
									if (already_taken == true) { 
										continue; 
									} else {
										// push back all neighbors of last element of the vector
										temp_neighbor_candidate.push_back(k);  						
									}
								}
							}
							if (initial_output==true){
								//myOutputFile1 << "temp_neighbor_candidate.size()=" << temp_neighbor_candidate.size() << endl;
								//cout << "temp_neighbor_candidate.size()=" << temp_neighbor_candidate.size() << endl;
							}
							// check to see if the last element of vector has at least one neighbor?
							if ( temp_neighbor_candidate.size() > 0 ) {
								// if yes, now pick one from the list of available neighbors
								int chosen_node = temp_neighbor_candidate[ rand() % temp_neighbor_candidate.size() ];
								// add the neighbor to the district
								initial[chosen_node][i] = 1;    
								//add the neighbor to single vector that we are using for traverse
								district_nodes[i].push_back(chosen_node); 

								// also do the same for ward's potential glued wards
								if( splitWards[chosen_node].size() > 0 ){
									for (int j =0; j< splitWards[chosen_node].size(); ++j) {

										//myOutputFile1 << j <<"----" << chosen_node << " : " <<splitWards[chosen_node][j] << endl; 
										initial[splitWards[chosen_node][j]][i] = 1;    
										//add the neighbor to single vector that we are using for traverse
										district_nodes[i].push_back(splitWards[chosen_node][j]); 
										//myOutputFile1 << "\nnode " << splitWards[chosen_node][j]<< " now assigned to dist. " << i; 
										//myOutputFile1 << " --  initial[" << splitWards[chosen_node][j]<< "][" << i << "] = 1";	
									}
								}



								// also push back any inside wards
								if(accompanyingWards[chosen_node].size() > 0){
									for (int j =0; j< accompanyingWards[chosen_node].size(); ++j) {
										//myOutputFile1 << j <<"----" <<accompanyingWards[chosen_node][j] << endl; 
										initial[accompanyingWards[chosen_node][j]][i] = 1; 
										district_nodes[i].push_back(accompanyingWards[chosen_node][j]);
										//myOutputFile1 << "\nnode " << accompanyingWards[chosen_node][j] << " now assigned to dist. " << i << " GLUED"; 
										//myOutputFile1 << " --  initial[" << accompanyingWards[chosen_node][j] << "][" << i << "] = 1";		

									}
								}



								if (initial_output==true){
									//myOutputFile1 << "\nnode " << chosen_node << " now assigned to dist. " << i; 
									//myOutputFile1 << " --  initial[" << chosen_node << "][" << i << "] = 1";
									//cout << "\nnode " << chosen_node << " now assigned to dist. " << i; 
									//cout << " --  initial[" << chosen_node << "][" << i << "] = 1";
								}

							} else {
								if (initial_output==true){
									//myOutputFile1 << "\nnode " << district_nodes[i].back() << " is a dead end! removing from traverse list. " << i;
									//cout << "\nnode " << district_nodes[i].back() << " is a dead end! removing from traverse list. " << i;
								}
								// if couldn't find a neighbor for last element of vector, delet it
								district_nodes[i].pop_back();
							}
						}
						//cout << " \n=================================================== ";
					} while (microdists_left == true);

					// memory cleanup?
					for (int ii = 0; ii < noDistricts; ii++) {
						district_nodes[ii].clear();
						vector<int>().swap(district_nodes[ii]);
					}
					vector<vector<int>>().swap(district_nodes);
					vector<int>().swap(district_seeds);
					// -- END: dynamic initialization of districts based on traversing

					///////////Test/////////////////////////////////////
					if (initial_output==true){
						int sum; 
						for (int i=0; i< numSubs; i++) {
							sum=0;
							for (int j=0; j<noDistricts ; j++)
							{
								sum= sum + initial [i][j] ;
							}
							if (sum < 1) {
								listOfInia.push_back(i);
							}
						}
						//myOutputFile1 << "listOfInia.size "<<listOfInia.size() << ":" ;
						//myOutputFile1 << "unassigned nodes" << endl; 
						//cout << "listOfInia.size "<<listOfInia.size() << ":" ;
						//cout << "unassigned nodes" << endl; 
						for (auto ii=listOfInia.begin() ; ii< listOfInia.end() ; ii++){
							//myOutputFile1 << *ii << "," ; 
						}

						//myOutputFile1 << "neighbors=" << endl; 
						cout << "neighbors=" << endl; 
						for (auto ii=listOfInia.begin() ; ii< listOfInia.end() ; ii++){
							for (int j=0; j< numSubs; j++) { 
								if (adjacency[*ii][j]==1){
									listOfInib.push_back(j);
									//myOutputFile1 << "neighbor["<<*ii << " ]:" << j << "," ;
									cout << "neighbor["<<*ii << " ]:" << j << "," ;
								}
							}
							listOfInib.clear();
						}

						////////////////////////////////////
						// Print Current Districs
						/*
						for (i=0;i<numSubs;i++)
						{
						for (j=0;j<noDistricts;j++)
						{
						//myOutputFile << initial[i][j] << ",";
						if (initial [i][j]==1){
						myOutputFile << i << "  " << j << endl; 
						}
						}
						//myOutputFile << endl;
						}
						//*/
						/////////////Test/////////////////
						int cn =0; 
						int Tcn =0;
						for (i=0;i<noDistricts;i++)
						{
							cn=0; 
							for (j=0;j<numSubs;j++){
								if ( initial [j][i] ==1) {
									cn=cn+1;
								}
							}
							//myOutputFile1 << " Count col ["<< i << "]=" << cn  << endl ;
							cout << " Count col ["<< i << "]=" << cn  << endl ;
							Tcn = Tcn+cn;
						}
						//myOutputFile1 << "Tcn "<< Tcn << endl; 
						cout << "Tcn "<< Tcn << endl; 
					}
					////////////////////////////////////////////////////////////////////////////////////////////
					// find the first Penalty for each generated district
					distPop.resize(numSubs+1);
					distVote.resize(numSubs+1);
					distDem.resize(numSubs+1);
					distPerimeter.resize(numSubs+1); 
					distArea.resize(numSubs+1);
					//distDGap.resize(numSubs+1);
					//distRGap.resize(numSubs+1);

					Rep=0; // # of Rep seats
					Dem=0; // # of Dem seats			
					double P;  // district population 
					double D;  // democrats population  per district
					double R;  // Republicans population  per district			 
					double Temp; // for perimeter
					double sum; // for area
					double G;   //50 % +1  of district population
					GapD=0;  // Democrats gap
					GapR=0; //Republicans gap
					sumDGap=0;
					sumRGap=0;
					int Res=0;
					int r;

					for (int i = 0; i < noDistricts; i++)
					{

						// computing distPop[i]
						P = 0;  // Population of each district
						D = 0;  // Democrats of each district
						R = 0;  // Republicans of each district
						r=0;
						sum=0; // for area
						Temp=0; // for perimeter

						for (int j = 0; j < numSubs ; j++ )
						{
							int c=0;
							// Penalty for population
							P += (subPop[j]*initial[j][i]); 
							//Penalty for deviation of democrate vote
							D += (democrats[j]*initial[j][i]);
							R += ((voter[j]-democrats[j])*initial[j][i]);

							//// Penalty for compactness 
							//finding the perimeter of each district					
							if (initial[j][i]>0.99) 
							{
								for (int k=0; k<numSubs; k++)
								{
									if ( initial [k][i]==1 && adjacency[j][k]> 0.99) 
									{                   
										c +=bLength[j][k];      
									}
								}
							}
							// finding perimeter of each district

							Temp = Temp + perimeter[j]-c; 
							// finding area of each district
							sum = sum + area[j] * initial[j][i];


							if (initial[j][i]>0.99) 
							{
								for (auto ii=Address.begin() ; ii < Address.end() ; ii++){
									if (j==*ii){
										r=r+1;
									}
								}
							}

						}

						// Efficiency Gap (based on second definition of political fairness)
						/*
						G = ceil (0.50* (R+D));
						if (D >= G) {
						GapR = R ; 
						GapD = D - G;
						}
						else { 
						GapD = D ; 
						GapR = R - G;
						}
						//*/

						// Parties' Seats (based on third definition of political fairness)
						//*
						if (R > D){
							Rep=Rep+1;
						}
						else{
							Dem=Dem+1;  
						}
						//*/

						//cout << " GapD , GapR" << GapD << ","<< GapR<< endl;        

						//(1)Pop penalty for each district
						distPop[i] = abs(P-Pbar);
						//distPop[i] = abs(pow((P-Pbar),2));
						//distPop[i] = abs(pow((P-Pbar),3));

						//(2)Political fairness penalty for each district
						// based on first definition of political fairness
						//distDem[i] = abs(D-AvgNumDemocratsPerDistrict);
						// based on second definition of political fairness(Efficiency gap)
						// sumDGap += GapD; //Total Democrats' Gap (state wide)
						// sumRGap += GapR; //Total Republicans' Gap (state wide)

						//(3)Lack of compactness(Perimeter and Area)
						distPerimeter[i]=Temp;    
						distArea[i]=sum;
						if (r>0){
							Res=Res+r-1;
						}
					}

					if (initial_output==true){
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distPop" << i << "=" << distPop[i]<<endl; 
						cout << "distPop" << i << "=" << distPop[i]<<endl; 
						//for (i = 0; i < noDistricts ; i++)
						//	myOutputFile1 << "distDem" << i << "=" << distDem[i]<<endl;	
						//	cout << "distDem" << i << "=" << distDem[i]<<endl;	
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distPerimeter" << i << "=" << distPerimeter[i]<<endl;
						cout << "distPerimeter" << i << "=" << distPerimeter[i]<<endl;
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "distArea" << i << "=" << distArea[i]<<endl;
						cout << "distArea" << i << "=" << distArea[i]<<endl;
						for (i = 0; i < noDistricts; i++)
							//myOutputFile1<< "Compactness" << i << "=" << pow(distPerimeter[i],2)/distArea[i]<<endl;
						cout << "Compactness" << i << "=" << pow(distPerimeter[i],2)/distArea[i]<<endl;
					}

					for (i= 0; i < noDistricts; i++) {               
						OriginalFirstPenaltyP += 1000*(distPop[i]- meanP)/SdP;   
						//OriginalFirstPenaltyV += 1000* ( distDem[i]-meanV)/SdV;
						OriginalFirstPenaltyCom += 1000* (pow(distPerimeter[i],2)/distArea[i]-meanC)/SdC;
					}

					//*
					//beforeNormalFirstVote = abs((sumDGap)/VD-(sumRGap)/VR)/2; // Political Fairness for efficiency gap (second definition)
					beforeNormalFirstVote = abs(Rep - ceil ((1-Vote)*noDistricts)); // Political Fairness for seats (third definition)
					//OriginalFirstPenaltyV= (beforeNormalFirstVote); // normalized
					OriginalFirstPenaltyV= 10000*(beforeNormalFirstVote-meanV)/SdV; // normalized


					beforeNormalFirstRes = Res;
					OriginalFirstPenaltyR=1000*(beforeNormalFirstRes-meanR)/SdR;



					//cout << "OriginalFirstPenaltyP=" << OriginalFirstPenaltyP << endl;
					//cout << "OriginalFirstPenaltyV=" << OriginalFirstPenaltyV << endl;
					//cout << "OriginalFirstPenaltyCom=" << OriginalFirstPenaltyCom << endl;
					//cout << "OriginalFirstPenaltyR=" << OriginalFirstPenaltyR << endl;

					//*/

					//weighted normalized penalty 
					firstPenalty = w1* OriginalFirstPenaltyP + w2* OriginalFirstPenaltyV + w3* OriginalFirstPenaltyCom+w4*OriginalFirstPenaltyR;
					//cout << "firstPenalty=" << firstPenalty << endl;
					if (firstPenalty < bestbefnormpen){				

						for (i=0; i < numSubs; i++){
							for(j=0; j<noDistricts; j++)
							{
								binitial[i][j]=initial[i][j];
								//cout << "done" << endl; 
							}
						}
						bestbefnormpen=firstPenalty;				
					}

					//*
					if (initial_output==true){
						//myOutputFile << "iniGenNumber=" << iniGenNumber << endl; 
						cout << "iniGenNumber=" << iniGenNumber << endl; 
					}
				} while (iniGenNumber < noInitialGeneration);  // for generating 100 initials 

				///////////////////////////////////////////////////////
				// solution of Best initial districts ***************//
				///////////////////////////////////////////////////////
				if (detailed_output==true){
					cout << "informaton of the best initial districts" << endl; 
				}
				// find the first Penalty for best initial solution
				distFPop.resize(numSubs+1);
				distFVote.resize(numSubs+1);
				distFDem.resize(numSubs+1);
				distFArea.resize(numSubs+1);
				distFPerimeter.resize(numSubs+1);  

				firstPenalty=0;  // Weighted Normalized total penalty
				firstPenaltyP=0; // Weighted Normalized total Pop penalty
				firstPenaltyV=0; // Weighted Normalized total Political fairness penalty
				firstPenaltyCom=0; // Weighted Normalized total compactness penalty
				firstPenaltyR=0;

				OriginalFirstPenalty=0;  // unWeighted Normalized total penalty
				OriginalFirstPenaltyP=0; //unWeighted Normalized total Pop penalty
				OriginalFirstPenaltyV=0; // unWeighted Normalized total Political fairness penalty
				OriginalFirstPenaltyCom=0; // unWeighted Normalized total compactness penalty
				OriginalFirstPenaltyR=0;

				beforeNormalFirstPenalty=0;//Unweighted not Normalized total penalty
				beforeNormalFirstPop=0;  //unWeighted Normalized total Pop penalty
				beforeNormalFirstVote=0;  // unWeighted Normalized total Political fairness penalty
				beforeNormalFirstCom=0;// unWeighted Normalized total compactness penalty
				beforeNormalFirstRes=0; 

				double Pdist;  // district population 
				//distDGap.resize(numSubs+1);
				//distRGap.resize(numSubs+1);
				double Ddist;  // democrats population  per district
				double Rdist;  // Republicans population  per district
				double Temp; 
				double sum;
				double G;   //50 % +1  of district population
				GapD=0;  // Democrats gap
				GapR=0; //Republicans gap
				double sumFDGap=0;
				double sumFRGap=0;
				int FRep=0; // # of Rep seats
				int FDem=0; // # of Dem seats
				int FRes;
				int r;

				for (int i = 0; i < noDistricts; i++)
				{

					//computing distPop[i]
					Pdist = 0;  // Population of each district
					Ddist = 0;  // Democrats of each district
					Rdist = 0;  // Republicans of each district
					sum=0;
					r=0;
					Temp=0; 

					for (int j = 0; j < numSubs ; j++ )
					{	
						int c=0; 
						//Penalty for population
						Pdist += (subPop[j]*binitial[j][i]); 
						//Penalty for deviation of democrate vote
						Ddist += (democrats[j]*binitial[j][i]);
						Rdist += ((voter[j]-democrats[j])*binitial[j][i]);
						//Penalty for lack of compactness 
						//finding the perimeter of each district
						//*
						if (binitial[j][i]>0.99) 
						{
							for (int k=0; k<numSubs; k++)
							{
								if ( binitial [k][i]==1 && adjacency[j][k]> 0.99) 
								{                   
									c +=bLength[j][k];     
								}
							}
						}
						// finding perimeter of each district

						Temp = Temp + perimeter[j]-c; 
						// finding area of each district
						sum = sum + area[j] * binitial[j][i];


						if (binitial[j][i]>0.99) 
						{
							for (auto ii=Address.begin() ; ii < Address.end() ; ii++){
								if (j==*ii){
									r=r+1;
								}
							}
						}
					}

					// store the information of each district in two 2-dim districts
					Popvect.push_back(Pdist);
					Demvect.push_back(Ddist);
					Repvect.push_back(Rdist);
					Areavect.push_back(sum);
					Perivect.push_back(Temp);
					TPopvect.push_back(Pdist);
					TDemvect.push_back(Ddist);
					TRepvect.push_back(Rdist);
					TAreavect.push_back(sum);
					TPerivect.push_back(Temp);

					// Efficiency Gap (based on second definition of political fairness)
					/*
					G = ceil (0.50* (Rdist+Ddist));
					if (Ddist >= G) {
					GapR = Rdist ; 
					GapD = Ddist - G;
					}
					else { 
					GapD = Ddist ; 
					GapR = Rdist - G;
					}

					RGapvect.push_back(GapR);
					TRGapvect.push_back(GapR);
					DGapvect.push_back(GapD);
					TDGapvect.push_back(GapD);
					//*/
					// Parties' Seats (based on third definition of political fairness)
					//*
					if (Rdist > Ddist){
						FRep=FRep+1;
					}
					else{
						FDem=FDem+1;  
					}
					//*/     

					//(1)Pop penalty for each district
					distFPop[i] = abs(Pdist-Pbar);
					//distPop[i] = abs(pow((P-Pbar),2));
					//distPop[i] = abs(pow((P-Pbar),3));

					//(2)Political fairness penalty for each district
					//1) based on first definition of political fairness
					//distFDem[i] = abs(Ddist-AvgNumDemocratsPerDistrict);

					// 2) based on third definition of political fairness(Efficiency gap)
					// sumFDGap += GapD; //Total Democrats' Gap (state wide)
					// sumFRGap += GapR; //Total Republicans' Gap (state wide)

					//(3)Lack of compactness(Perimeter and Area)
					distFPerimeter[i]=Temp;    
					distFArea[i]=sum;
					if (r>0){
						FRes=FRes+r-1;
					}

					if(detailed_output==true){
						cout << "district["<< i <<"]: PopDeviation=" <<abs(Pdist-Pbar)/ Pbar << " and population= " << Pdist << " and ratio of democrats= " << Ddist/(Rdist+Ddist) << " and democrats's gap= " << GapD <<" and Republicans' gap=" << GapR << " and compactness=" << (pow((Temp),2)/sum) << endl; 
					}
				}
				///////////////////////////////////////////////////////////////////		
				/*       
				// For Effciency Gap and # of seats:

				double TempMeanVE=0; 
				for (auto ii=listOfmeanV.begin() ; ii< listOfmeanV.end() ; ii++){
				TempMeanVE += *ii; 
				}
				meanVE= TempMeanVE/ listOfmeanVE.size() ; 

				for (auto ii=listOfmeanV.begin() ; ii< listOfmeanV.end() ; ii++){
				SdVE +=  pow((listOfmeanV[*ii] - meanVE), 2);
				}
				SdVE=sqrt(SdVE/listOfmeanVE.size()); 
				//*/	

				// Find the penalty for each districts 
				PenaltyP.resize(numSubs+1);
				PenaltyV.resize(numSubs+1);
				PenaltyCom.resize(numSubs+1);
				PenaltyR.resize(numSubs+1);
				OriginalFPenaltyP.resize(numSubs+1);
				OriginalFPenaltyV.resize(numSubs+1);
				OriginalFPenaltyCom.resize(numSubs+1);
				OriginalFPenaltyR.resize(numSubs+1);

				// I multiplied each penalty by 1000 to ease the SA functionality

				for (i= 0; i < noDistricts; i++) {               
					OriginalFPenaltyP[i] = 1000 * (((distFPop[i]- meanP)/SdP));   
					//OriginalFPenaltyV[i] =  1000 * (((distFDem[i]-meanV)/SdV));
					OriginalFPenaltyCom[i] =  1000 * (((pow(distFPerimeter[i],2)/distFArea[i])-meanC)/SdC);
				}

				for (i= 0; i < noDistricts; i++) {   
					//Original Not normalized penalty (Pure)
					beforeNormalFirstPop += distFPop[i];
					//beforeNormalFirstVote += distFDem[i];
					beforeNormalFirstCom += pow((distFPerimeter[i]),2)/distFArea[i];

					// Original normalized Penalty 
					//OriginalFirstPenalty += OriginalPenalty[i];
					OriginalFirstPenaltyP += OriginalFPenaltyP[i];
					//OriginalFirstPenaltyV += OriginalFPenaltyV[i];
					OriginalFirstPenaltyCom += OriginalFPenaltyCom[i];
				}

				//*
				// Political Fairness for efficiency gap and parties's seats
				//beforeNormalFirstVote =  abs((sumDGap)/VD-(sumRGap)/VR)/2;  //Political Fairness for efficiency gap 
				beforeNormalFirstVote = abs(FRep - ceil ((1-Vote)*noDistricts));  //Political Fairness for parties's seats
				OriginalFirstPenaltyV= 10000*(beforeNormalFirstVote-meanV)/SdV;
				//OriginalFirstPenaltyV= (beforeNormalFirstVote);
				firstPenaltyV = w2*OriginalFirstPenaltyV;

				//*/


				beforeNormalFirstRes = FRes;  //Political Fairness for parties's seats
				OriginalFirstPenaltyR= 1000*(beforeNormalFirstRes-meanR)/SdR;
				firstPenaltyR = w2*OriginalFirstPenaltyR;



				// Weighted Normalized penalty
				firstPenaltyP = w1*OriginalFirstPenaltyP;
				firstPenaltyV = w2*OriginalFirstPenaltyV;
				firstPenaltyCom = w3* OriginalFirstPenaltyCom;
				firstPenaltyR = w4* OriginalFirstPenaltyR;

				// Total Penalty 
				OriginalFirstPenalty = OriginalFirstPenaltyP + OriginalFirstPenaltyV + OriginalFirstPenaltyCom+OriginalFirstPenaltyR; // Normalized not weighted Penalty
				firstPenalty = firstPenaltyP + firstPenaltyV + firstPenaltyCom+firstPenaltyR;  // Weighted Normalized Penalty
				beforeNormalFirstPenalty= w1* beforeNormalFirstPop + w2* beforeNormalFirstVote + w3* beforeNormalFirstCom+w4*beforeNormalFirstRes; // Weighted not normalized Penalty

				//myOutputFile << "firstPenalty~~~~~~~~~~~" << firstPenalty << endl; 
				double firstPenaltyStore=(beforeNormalFirstPop+beforeNormalFirstVote)/10; //Temprory 

				//cout << "OriginalFirstPenaltyP=" << OriginalFirstPenaltyP << endl;
				//cout << "OriginalFirstPenaltyV=" << OriginalFirstPenaltyV << endl;
				//cout << "OriginalFirstPenaltyCom=" << OriginalFirstPenaltyCom << endl;
				//cout << "OriginalFirstPenaltyR=" << OriginalFirstPenaltyCom << endl;
				//myOutputFile << "firstPenalty~~~~~~~~~~~" << firstPenalty << endl; 

				if (initial_output==true){			
					myOutputFile << "firstPenalty: " << firstPenalty << endl; 
					cout << " \n ** PENALTY: " << firstPenalty;
					/*
					for (i=0; i<numSubs; i++)
					{
						for (j=0; j<noDistricts; j++)
						{
							//myOutputFile2 << initial[i][j] << ",";
							if (binitial[i][j]==1){
								myOutputFile << i << " " << j ; 
							}
						}
						myOutputFile << endl;
					}
					//*/
					/*
					for (i=0; i<numSubs; i++)
					{
					for (j=0; j<noDistricts; j++)
					{
					if(binitial[i][j]==1){
					//cout << "ward["<< i <<"]is assigned to district["<< j <<"];" <<endl; 
					}
					}
					}
					//*/
					//cout << "firstPenalty="<< firstPenalty << endl;
					//cout << "normalizedWeightedPenaltyP="<< firstPenaltyP << endl;
					//cout << "normalizedWeightedPenaltyV="<< firstPenaltyV<< endl;
					//cout << "normalizedWeightedPenaltyCom="<< firstPenaltyCom << endl;

					//cout << "unWeighted Normalized total penalty="<< OriginalFirstPenalty << endl;
					//cout << "unWeighted Normalized Pop penalty="<< OriginalFirstPenaltyP << endl;
					//cout << "unWeighted Normalized Political penalty="<< OriginalFirstPenaltyV << endl;
					//cout << "unWeighted Normalized compactness penalty="<< OriginalFirstPenaltyCom << endl;

					//cout << "unWeighted not Normalized total penalty="<< beforeNormalFirstPenalty << endl;
					//cout << "unWeighted not Normalized Pop penalty="<< beforeNormalFirstPop << endl;
					//cout << "unWeighted not Normalized Political penalty="<< beforeNormalFirstVote << endl;
					//cout << "unWeighted not Normalized compactness penalty="<< beforeNormalFirstCom << endl;
				}

				myOutputFile << "firstPenalty="<< firstPenalty << endl;
				myOutputFile << "normalizedWeightedPenaltyP="<< firstPenaltyP << endl;
				myOutputFile << "normalizedWeightedPenaltyV="<< firstPenaltyV<< endl;
				myOutputFile << "normalizedWeightedPenaltyCom="<< firstPenaltyCom << endl;
				myOutputFile << "normalizedWeightedPenaltyR="<< firstPenaltyR << endl;

				myOutputFile << "unWeighted Normalized total penalty="<< OriginalFirstPenalty << endl;
				myOutputFile << "unWeighted Normalized Pop penalty="<< OriginalFirstPenaltyP << endl;
				myOutputFile << "unWeighted Normalized Political penalty="<< OriginalFirstPenaltyV << endl;
				myOutputFile << "unWeighted Normalized compactness penalty="<< OriginalFirstPenaltyCom << endl;
				myOutputFile << "unWeighted Normalized Residency penalty="<< OriginalFirstPenaltyR << endl;

				myOutputFile << "unWeighted not Normalized total penalty="<< beforeNormalFirstPenalty << endl;
				myOutputFile << "unWeighted not Normalized Pop penalty="<< beforeNormalFirstPop << endl;
				myOutputFile << "unWeighted not Normalized Political penalty="<< beforeNormalFirstVote << endl;
				myOutputFile << "unWeighted not Normalized compactness penalty="<< beforeNormalFirstCom << endl;
				myOutputFile << "unWeighted not Normalized Residency penalty="<< beforeNormalFirstRes << endl;

				///////////////////////////////////////////////	 
				//find the edges of initial ****************///
				//////////////////////////////////////////////
				vector<int> myCorners , myCoreNodes;
				//	cout << "Finding corners of dist: ";
				for (int ii=0; ii < noDistricts; ii++ ){
					//	cout << ii << " ";
					myCorners.clear();
					myCoreNodes.clear();
					for (int jj=0; jj < numSubs; jj++ ){
						if (binitial[jj][ii] == 1) {
							myCorners.push_back(jj);
							//cout << "My jj is &&&&&&&&&&" << jj << endl
						}
					}
					district_wards.push_back(myCorners);
					Tempdistrict_wards.push_back(myCorners);

					//for (auto i = myCorners.begin(); i != myCorners.end(); i++){
					for(auto i = district_wards[ii].begin(); i!=district_wards[ii].end(); i++){
						//cout << "My i is ============" << *i << endl; 
						for (int kk=0 ; kk< numSubs; kk++) {
							if (adjacency [*i][kk]== 1){

								// if kk sub isn't in ii dist, push it to myCoreNodes
								//if (binitial[kk][ii]==0){
								if(std::find(district_wards[ii].begin(), district_wards[ii].end(), kk) == district_wards[ii].end()){							
									myCoreNodes.push_back(*i);
									break;
								}

							}				
						}

					}
					corners_nodes.push_back(myCoreNodes);
				}

				if (edges_output==true){
					//myOutputFile << "\n\n corners_nodes: "; 
					cout << "\n\n edges_nodes: "; 
					for (int i = 0; i < corners_nodes.size(); i++)  {  
						//myOutputFile << "wards of district " << i << endl; 
						cout << "wards of district " << i << endl; 
						for (int j = 0; j < district_wards[i].size(); j++) {     
							//myOutputFile1 << district_wards[i][j] << ", ";
							cout << district_wards[i][j] << ", ";
						}
						cout << endl; 
						//myOutputFile << "edges of district " << i << endl;
						cout << "updated edges of district " << i << endl;
						for (int j = 0; j < corners_nodes[i].size(); j++) {     
							//myOutputFile << corners_nodes[i][j] << ", ";
							cout << corners_nodes[i][j] << ", ";
						}
						cout << endl; 
					}
				}	



				/////////////////////////////////////////////	 
				//find each ward's district***************///
				//////////////////////////////////////////////

				for (int ii=0; ii < numSubs; ii++ ){
					myCorners.clear();
					for (int jj=0; jj < noDistricts; jj++ ){
						if (binitial[ii][jj] == 1) {
							myCorners.push_back(jj);
							break; 
						}
					}
					ward_districts.push_back(myCorners);		
				}


				////////////////////////////////////////////////////////////////////////////////////////////////////////////
				///////////////////neighbors of each wards//////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////////////////
				neighbors.resize(numSubs);
				for(i=0; i<numSubs; i++){			
					for(j=0; j<numSubs; j++){					
						if(adjacency[i][j]==1){						
							neighbors[i].push_back(j); 
						}
					}				
				}
				//for (j=0; j<neighbors[4250].size(); j++) {
				//cout << "neighbors[4250]" << neighbors[4250][j] << endl; 
				//}




				//////////////////////////////////////////////////////////////////////////

				//*

				int el;
				for(i=0; i<numSubs; i++){
					//cout << "i----------------: " << i << endl; 
					for (auto j = MySplit[i].begin(); j != MySplit[i].end(); ++j) {
						el=0;
						//cout << "j----------------: " << *j << endl; 
						for (auto k = neighbors[*j].begin(); k != neighbors[*j].end(); ++k) {
							//cout << "k----------------: " << *k << endl; 
							if(std::find( MySplit[i].begin(),  MySplit[i].end(), *k) ==  MySplit[i].end() && *k != i){
								continue;
							}
							else{
								el=el+1;
							}
						}
						if (el==0){
							cout<< "Suspect Splt Ward: "<< *j << endl;
						}
					}
				}


				//*/

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//double currentDistPenalty = firstPenalty;
				double bestDistPenalty = firstPenalty;
				double bestDistPenaltyP = firstPenaltyP;
				double bestDistPenaltyV = firstPenaltyV;
				double bestDistPenaltyCom = firstPenaltyCom;
				double bestDistPenaltyR = firstPenaltyR;

				//double OriginalcurrentDistPenalty = OriginalFirstPenalty;
				double OriginalBestDistPenalty = OriginalFirstPenalty;
				double OriginalBestDistPenaltyP = OriginalFirstPenaltyP;
				double OriginalBestDistPenaltyV = OriginalFirstPenaltyV;
				double OriginalBestDistPenaltyCom = OriginalFirstPenaltyCom;
				double OriginalBestDistPenaltyR = OriginalFirstPenaltyR;

				double bestBeforeNormalPenalty=beforeNormalFirstPenalty ;
				double bestBeforeNormalP= OriginalFirstPenaltyP;
				double bestBeforeNormalV= OriginalFirstPenaltyV;
				double bestBeforeNormalCom= OriginalFirstPenaltyCom ;
				double bestBeforeNormalR= OriginalFirstPenaltyR;

				double nextDistPenalty;  //normalizedWeightedNextPenalty
				double nextDistPenaltyP; //normalizedWeightedNextPopulationPenalty
				double nextDistPenaltyV; //normalizedWeightedNextPliticalFairnessPenalty
				double nextDistPenaltyCom; //normalizedWeightedNextCompactnessPenalty
				double nextDistPenaltyR; 

				double OriginalNextDistPenalty; //normalizedUnWeightedNextPenalty
				double OriginalNextDistPenaltyP; //normalizedUnWeightedNextPopulationPenalty
				double OriginalNextDistPenaltyV; //normalizedUnWeightedNextPliticalFairnessPenalty
				double OriginalNextDistPenaltyCom; //normalizedUnWeightedNextCompactnessPenalty
				double OriginalNextDistPenaltyR; 

				double beforeNormalPenalty;//notNormalizedUnWeightedNextPenalty
				double beforeNormalPop; //notNormalizedUnWeightedNextPopulationPenalty
				double beforeNormalVote; //notNormalizedUnWeightedNextPliticalFairnessPenalty
				double beforeNormalCom; //notNormalizedUnWeightedNextCompactnessPenalty
				double beforeNormalRes; 


				//////////////////////////////////////////////
				//The SA algorithm **********************/////
				/////////////////////////////////////////////
				int giving_node; 
				int giving_node_inside; 
				int giving_district; 
				int taking_node; 
				int taking_node_inside; 
				int taking_district; 
				int xx=0;
				int inside; 
				int splt;
				//Time Managing 
				double bestTime;  // when we find best answer
				double runTime=0; // Algorithm running time

				unsigned long int neighbortime=0; // Total time for finding the neighbors
				unsigned long int neighbortime00=0; 
				unsigned long int neighbortime01=0; 
				unsigned long int neighbortime02=0; 
				unsigned long int neighbortime03=0; 
				unsigned long int contiguityTime=0; // Total time for checking contiguity 
				unsigned long int contiguityTime00=0; 
				unsigned long int contiguityTime01=0;
				unsigned long int contiguityTime02=0;
				unsigned long int contiguityTime03=0;
				unsigned long int feasibleTime=0;  // Total  time for finding neighbors and checking contiguity (checking feasiblity)
				unsigned long int feasibleTime00=0; 
				unsigned long int feasibleTime01=0; 
				unsigned long int feasibleTime02=0; 
				unsigned long int feasibleTime03=0; 
				unsigned long int Penaltycaltime=0; // Total time for penalty calculation
				unsigned long int Penaltycaltime00=0;
				unsigned long int Penaltycaltime01=0;
				unsigned long int Penaltycaltime02=0;
				unsigned long int Penaltycaltime03=0;
				unsigned long int AcceptanceTime=0; // Total time for evaluation of answers
				unsigned long int AcceptanceTime00=0;
				unsigned long int AcceptanceTime01=0;
				unsigned long int AcceptanceTime02=0;
				unsigned long int AcceptanceTime03=0;
				vector<double> HighPopNodes; // high population deviation
				vector<double> HighPop; // high population
				vector<double> LowPopNodes; // low population deviation
				//vector<double> HighGapNodes; // high eff gap
				//vector<double> LowGapNodes;  // low eff gap

				vector<int> MyKK , MyNN, Mygt, Mygg, Mytg, Mytt; // Vectors for the neighbors of giving and taking nodes

				vector<int> giving_node_split  , taking_node_split ;
				vector<int> other_contiguity_check;
				//vector<int> giveSpltDist , takeSpltDist ;

				initialtime += (time(NULL)-myTime0);


				//////////////////////////////////////////////////////////////
				///Running Algorithm**************************////////////////
				/////////////////////////////////////////////////////////////
				//while (temperature > FinalTemp)  // We are using time limit so we no longer need this loop
				//{
				//cout << "FDem=" << FDem << endl; 
				//cout <<"FRep=" <<FRep  << endl; 
				while (runTime <  algorithmRunTime)  // we can eliminate this loop to be solved without time limit 
				{
					while (iterations < NUM_ITERATIONS) //Addind another loop to 
					{
						//If desired, display the current districts under investigation.//iterations == 0 || iterations == NUM_ITERATIONS - 1 
						if (1 == 0)
						{
							cout << "\t" << "Starting a new iteration...we've already made "
								<< iterations << " iterations and taken " << steps << " steps.  Current temperature = " << temperature << endl;
							cout << "\t" << "The current districts is: ";
							for (i=0;i<numSubs;i++)
								for (j=0;j<noDistricts;j++)
								{
									cout << binitial[j][i] << ",";
								}
								cout << endl;
								//cout << "\t" << "CurrentdistPenalty = " << currentDistPenalty << endl;
						}
						///////////////////////////////////////////////////////////////////////////


						long int myTime1 = time(NULL); // for feasiblity
						int Cont=0;


						//Setting up the next solution

						do {
							long int myTime2 = time(NULL); //for finding neighbor
							long int myTime2_00 = time(NULL);
							reject = false;

							districts_to_check_contiguity.clear();
							other_contiguity_check.clear();
							noOfGeneration=noOfGeneration+1;
							//////////////////////////////////////////////////////////////////////////////////////
							// find a random micro district from a random district and give it to its own neighbor distric which has a common boundry with selected micro-district
							if ((noOfGeneration%10)>8){
								cout << "listsizeHP=" << listsizeHP << endl;
							}
							int q;
							MyKK.clear();
							MyNN.clear();	
							Mygg.clear();	
							Mygt.clear();	
							Mytg.clear();	
							Mytt.clear();	
							giving_node_split.clear();
							taking_node_split.clear();
							//giveSpltDist.clear();
							//takeSpltDist.clear();


							/*
							//Sort district whith high population deviation based on their population
							vector< pair <double, int >> vect; // Pair vector
							//pair sort based on population
							for (int i=0; i<HighPopNodes.size(); i++)
							vect.push_back(make_pair(HighPop[i], HighPopNodes[i]));
							//std::sort(LowGapNodes.begin(), LowGapNodes.end());
							std::sort(vect.begin(), vect.end(), sortinrev);
							//std::reverse(vect.begin(), vect.end());
							/*
							for (int i=0; i<HighPopNodes.size(); i++)
							{
							cout << vect[i].first << "     "
							<< vect[i].second << endl;
							}
							//*/
							int r= rand()%10; 
							// find giving and taking districts and nodes
							if (listsizeHP >= 1 ) {

								/*
								//int r= rand()%10; 
								if ( listsizeHP < 10){	
								q= rand() % listsizeHP;
								giving_district = vect[q].second;
								//giving_district = HighPopNodes[rand() % listsizeHP];
								}
								else{
								//if (r<5){
								//q= rand() % abs(listsizeHP/(rand()%8+1));
								q= rand() % 10;
								giving_district = vect[q].second;
								//	}
								//	else{
								//		q= rand() % abs(listsizeHP/10);
								//		giving_district = vect[q].second;
								//	}
								}
								//*/
								//giving_district = vect[q].second;
								if (r>3){
									giving_district = HighPopNodes[rand() % listsizeHP];
								}
								else{
									giving_district = rand() % noDistricts;	
								}
							}
							else {						
								giving_district = rand() % noDistricts;						
							}
							//*

							giving_node=corners_nodes[giving_district][rand() % corners_nodes[giving_district].size()]; 
							//cout << "giving_node" << giving_node << endl; 
							//cout << "giving_district" << giving_district << endl; 


							inside=-1;
							splt=-1;

							if (Popvect[giving_district] > Pbar ){

								if(accompanyingWards[giving_node].size()>0){
									giving_node_inside=accompanyingWards[giving_node][0];
									inside=1;
								}
								//	cout << "giving_node" << giving_node << endl; 
								//*
								if(splitWards[giving_node].size()>0){
									for (auto i = splitWards[giving_node].begin(); i != splitWards[giving_node].end(); ++i) {
										//cout << "splitWards[giving_node]" << *i << endl; 
										//cout <<"splitWards[giving_node]_dist=" << ward_districts[*i][0] << endl; 
										giving_node_split.push_back(*i);

									}
									splt=1;
								}
							}

							if (splt==1){
								for (auto i = giving_node_split.begin(); i != giving_node_split.end(); ++i) {
									other_contiguity_check.push_back(ward_districts[*i][0]);
									//giveSpltDist.push_back(ward_districts[*i][0]);
								}

							}


							neighbortime00 += (time(NULL)-myTime2_00);
							long int myTime2_01 = time(NULL);
							////////////////////////////////////////////////////

							//////////////////////////////////////////////////////

							for (auto i = neighbors[giving_node].begin(); i!= neighbors[giving_node].end(); ++i) {
								//cout << *i << endl; 
								if(std::find(district_wards[giving_district].begin(), district_wards[giving_district].end(), *i) == district_wards[giving_district].end() ){
									//cout << *i << endl; 
									MyKK.push_back(*i);	
								}
								else{
									continue;
								}
							}

							/////////////////////////////////////////
							//*
							if (splt==1){
								for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
									//MyKK.erase(std::remove(begin(MyKK), end(MyKK), *k), end(MyKK));
									for (auto i = neighbors[*k].begin(); i!= neighbors[*k].end(); ++i) {
										//if(std::find(other_contiguity_check.begin(), other_contiguity_check.end(), ward_districts[*i][0]) == other_contiguity_check.end() && ward_districts[*i][0]!=giving_district ){
										if(std::find(district_wards[ward_districts[*k][0]].begin(), district_wards[ward_districts[*k][0]].end(), *i) == district_wards[ward_districts[*k][0]].end() && ward_districts[*i][0]!=giving_district ){
											//if(std::find(giving_node_split.begin(), giving_node_split.end(), *i) == giving_node_split.end() && *i!=giving_node ){
											//cout << *i << endl; 
											MyKK.push_back(*i);	
											//}
										}
										else{
											continue;
										}
									}

								}
							}
							//*/
							///////////////////////////////////////////

							//*/  
							/*
							cout << endl; 
							cout <<"MyKK _before:" << endl; 
							for (auto k = MyKK.begin(); k!= MyKK.end(); ++k)
							{
							cout << *k << ", " ; 
							}
							//*/
							//cout <<"MyKK.size()1=" << MyKK.size() << endl; 	
							std::sort(MyKK.begin(), MyKK.end());
							MyKK.erase(std::unique(MyKK.begin(), MyKK.end()), MyKK.end());

							cout << endl; 
							//cout <<"MyKK.size()2=" << MyKK.size() << endl; 	
							neighbortime01 += (time(NULL)-myTime2_01);
							long int myTime2_02 = time(NULL);
							////////////////////////////////////////////////////////
							//*
							taking_node = MyKK[rand() % (MyKK.size())]; 					
							taking_district=ward_districts[taking_node][0];
							//cout << "taking_node=" << taking_node << endl; 
							//cout <<"taking_district="<< taking_district<< endl; 
							//*/	


							if (Popvect[giving_district] <= Pbar ){
								if(accompanyingWards[taking_node].size()>0){
									taking_node_inside=accompanyingWards[taking_node][0];
									inside=2;
								}

								//*
								if(splitWards[taking_node].size()>0){
									for (auto i = splitWards[taking_node].begin(); i != splitWards[taking_node].end(); ++i) {
										//if (*i != giving_node){
										//cout << "splitWards[taking_node]=" << *i << endl; 
										//cout <<"splitWards[taking_node]_dist=" << ward_districts[*i][0] << endl; 
										taking_node_split.push_back(*i);
										//}
									}
									splt=2;
								}
							}

							std::sort(giving_node_split.begin(), giving_node_split.end());
							giving_node_split.erase(std::unique(giving_node_split.begin(), giving_node_split.end()), giving_node_split.end());
							std::sort(taking_node_split.begin(), taking_node_split.end());
							taking_node_split.erase(std::unique(taking_node_split.begin(), taking_node_split.end()), taking_node_split.end());


							if (splt==1){
								giving_node_split.erase(std::remove(begin(giving_node_split), end(giving_node_split), taking_node), end(giving_node_split));
							}


							if (splt==2){
								taking_node_split.erase(std::remove(begin(taking_node_split), end(taking_node_split), giving_node), end(taking_node_split));

							}



							if (splt==2){
								for (auto i = taking_node_split.begin(); i != taking_node_split.end(); ++i) {
									other_contiguity_check.push_back(ward_districts[*i][0]);
									//takeSpltDist.push_back(ward_districts[*i][0]);
								}

							}


							if (splt!=-1 && other_contiguity_check.size()>0){
								for (auto i = other_contiguity_check.begin(); i!= other_contiguity_check.end(); ++i) {
									districts_to_check_contiguity.push_back(*i);

								}
							}

							districts_to_check_contiguity.push_back(taking_district);
							districts_to_check_contiguity.push_back(giving_district);

							std::sort(districts_to_check_contiguity.begin(), districts_to_check_contiguity.end());
							districts_to_check_contiguity.erase(std::unique(districts_to_check_contiguity.begin(), districts_to_check_contiguity.end()), districts_to_check_contiguity.end());	

							
							neighbortime02 += (time(NULL)-myTime2_02);
							////////////////////////////////Test//////////////////////////////				

							long int myTime2_03 = time(NULL);

							if (Popvect[giving_district] > Pbar ){
								given=true; 
								//cout << "Done0" << endl; 
								if (neighbor_output==true){
									cout << "district [" << giving_district <<"] gives node ["<<giving_node <<"] to district ["<< taking_district <<"]" << endl;
								}
								// add the given node to taken district	
								Tempdistrict_wards[taking_district].push_back(giving_node);

								// remove given node from given district 
								Tempdistrict_wards[giving_district].erase(std::remove(begin(Tempdistrict_wards[giving_district]), end(Tempdistrict_wards[giving_district]), giving_node), end(Tempdistrict_wards[giving_district]));					

								if (inside==1){

									// add the given node to taken district	
									Tempdistrict_wards[taking_district].push_back(giving_node_inside);

									// remove given node from given district 
									Tempdistrict_wards[giving_district].erase(std::remove(begin(Tempdistrict_wards[giving_district]), end(Tempdistrict_wards[giving_district]), giving_node_inside), end(Tempdistrict_wards[giving_district]));					
								}	


								if (splt==1){
									for (auto i = giving_node_split.begin(); i != giving_node_split.end(); ++i) {

										// remove given node from given district 
										Tempdistrict_wards[ward_districts[*i][0]].erase(std::remove(begin(Tempdistrict_wards[ward_districts[*i][0]]), end(Tempdistrict_wards[ward_districts[*i][0]]), *i), end(Tempdistrict_wards[ward_districts[*i][0]]));	

										// add the given node to taken district	
										Tempdistrict_wards[taking_district].push_back(*i);

									}
								}	
								///////////////////////////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
								// find the neighbor of the giving node and giving dist and giving node and taking dist 

								for (auto i = district_wards[giving_district].begin(); i != district_wards[giving_district].end(); ++i) {

									if ( adjacency [*i][giving_node] == 1) {
										Mygg.push_back(*i);
									} 
								}

								for (auto i = district_wards[taking_district].begin(); i != district_wards[taking_district].end(); ++i) {


									if ( adjacency [*i][giving_node] == 1) {
										Mygt.push_back(*i);
									} 
								}

								if (splt==1){
									//*
									for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										for (auto i = district_wards[ward_districts[*k][0]].begin(); i != district_wards[ward_districts[*k][0]].end(); ++i) {

											if ( adjacency [*i][*k] == 1) {
												Mygg.push_back(*i);
											} 
										}

									}

									//*/
									for (auto i = district_wards[taking_district].begin(); i!= district_wards[taking_district].end(); ++i) 								
										for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
											if ( adjacency [*k][*i] == 1) {
												Mygt.push_back(*i);
											} 
										}
								}
							}
							///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							else
							{
								given=false; 

								if (neighbor_output==true){
									cout << "district ["<<taking_district<<"] gives node ["<<taking_node<<"] to district ["<<giving_district <<"]"<< endl;
								}
								//add the taken node to the low pop district
								Tempdistrict_wards[giving_district].push_back(taking_node);
								//remove the taken node from high pop district
								Tempdistrict_wards[taking_district].erase(std::remove(begin(Tempdistrict_wards[taking_district]), end(Tempdistrict_wards[taking_district]), taking_node), end(Tempdistrict_wards[taking_district]));

								if (inside==2){
									//add the taken node to the low pop district
									Tempdistrict_wards[giving_district].push_back(taking_node_inside);
									//remove the taken node from high pop district
									Tempdistrict_wards[taking_district].erase(std::remove(begin(Tempdistrict_wards[taking_district]), end(Tempdistrict_wards[taking_district]), taking_node_inside), end(Tempdistrict_wards[taking_district]));
								}

								if (splt==2){
									for (auto i = taking_node_split.begin(); i != taking_node_split.end(); ++i) {

										//remove the taken node from high pop district
										Tempdistrict_wards[ward_districts[*i][0]].erase(std::remove(begin(Tempdistrict_wards[ward_districts[*i][0]]), end(Tempdistrict_wards[ward_districts[*i][0]]), *i), end(Tempdistrict_wards[ward_districts[*i][0]]));	
										//add the taken node to the low pop district
										Tempdistrict_wards[giving_district].push_back(*i);

									}
								}	
								/////////////////////////////////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
								// find the neighbor of the taking node and giving dist and giving node and taking dist 								
								for (auto i = district_wards[giving_district].begin(); i != district_wards[giving_district].end(); ++i) {

									if ( adjacency [*i][taking_node] == 1) {
										Mytg.push_back(*i);
									} 
								}

								for (auto i = district_wards[taking_district].begin(); i != district_wards[taking_district].end(); ++i) {


									if ( adjacency [*i][taking_node] == 1) {
										Mytt.push_back(*i);
									} 
								}

								if (splt==2){

									for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										for (auto i = district_wards[ward_districts[*k][0]].begin(); i != district_wards[ward_districts[*k][0]].end(); ++i) {

											if ( adjacency [*i][*k] == 1) {
												Mytt.push_back(*i);
											} 
										}

									}


									for (auto i = district_wards[giving_district].begin(); i!= district_wards[giving_district].end(); ++i) 								
										for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
											if ( adjacency [*k][*i] == 1) {
												Mytg.push_back(*i);
											} 
										}
								}
							}

							//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							neighbortime03 += (time(NULL)-myTime2_03);
							neighbortime += (time(NULL)-myTime2);
							//////////////////////////////////////////////////////////////					
							if (neighbor_output==true){
								//*
								cout << "giving_district=" << giving_district << endl; 
								cout << "giving_node=" << giving_node << endl; 
								cout << "taking_district=" << taking_district << endl; 
								cout << "taking_node=" << taking_node << endl; 
								//*/
							}

							////////////////////////////////////////
							// Checking for contiguty*********//////
							////////////////////////////////////////

							long int myTime3 = time(NULL); //for checking contiguty
							long int myTime3_00 = time(NULL);
							if (contiguety_output==true){
								cout << endl << "districts_to_check_contiguity: " << endl;
							}
							for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) {

								subs_in_district.clear();
								traversed_nodes.clear();

								for (auto j = Tempdistrict_wards[*i].begin(); j!= Tempdistrict_wards[*i].end(); ++j) {
									subs_in_district.push_back(*j); 
									//cout << *j << " ";
								}												
								contiguityTime00 += (time(NULL)-myTime3_00);
								long int myTime3_01 = time(NULL);
								// start from the first sub and keep traversing nodes recursively
								traverseSubs( subs_in_district[0], adjacency );
								std::sort(traversed_nodes.begin(), traversed_nodes.end());
								std::sort(subs_in_district.begin(), subs_in_district.end());

								if (contiguety_output==true){
									//myOutputFile2 << "traversed_nodes rearranged: ";
									cout << "traversed_nodes rearranged: ";
									for (auto ii = traversed_nodes.begin(); ii != traversed_nodes.end(); ++ii) {    
										//myOutputFile2 << *ii << " ";
										cout << *ii << " ";
									}
									//myOutputFile2 << "subs_district rearranged: ";
									cout << "subs_district rearranged: ";
									for (auto ii = subs_in_district.begin(); ii != subs_in_district.end(); ++ii) {  
										//myOutputFile2 << *ii << " ";
										cout << *ii << " ";
									}
									//myOutputFile2 << endl;
									cout << endl;
								}

								contiguityTime01 += (time(NULL)-myTime3_01);

								long int myTime3_02 = time(NULL);
								if (traversed_nodes == subs_in_district) 
								{
									if (contiguety_output==true){
										//myOutputFile2 << " ** District " << *i << " Contiguous **" << endl;
										cout << " ** District " << *i << " Contiguous **" << endl;
									}
								}
								else {
									//Temprary Part //--------------------------------------------------------------------------------
									if(given==true){
										if(std::find(myNotContiguousWards.begin(), myNotContiguousWards.end(), giving_node) == myNotContiguousWards.end()){
											myNotContiguousWards.push_back(giving_node);
										}
									}
									else{
										if(std::find(myNotContiguousWards.begin(), myNotContiguousWards.end(), giving_node) == myNotContiguousWards.end()){
											myNotContiguousWards.push_back(taking_node);
										}
									}

									//--------------------------------------------------------------------------------------------------
									if (contiguety_output==true){
										//myOutputFile2 << " ** District " << *i << " ** NOT Contiguous! **" << endl;
										cout << " ** District " << *i << " ** NOT Contiguous! **" << endl;								
									}
									//cout << "No" << endl;	
									reject = true;		
									/////////////////////////////////////////
									// if it is not contiguet don't exchange the nodes
									//Tempdistrict_wards[taking_district]=district_wards[taking_district];
									//Tempdistrict_wards[giving_district]=district_wards[giving_district];

									for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) {
										Tempdistrict_wards[*i]=district_wards[*i];
									}


									////////////////////////////////////////
									break;
								}
								contiguityTime02 += (time(NULL)-myTime3_02);
							}
							cout << endl;

							contiguityTime += (time(NULL)-myTime3);

						} while (reject == true);   
						//	Cont=0;

						feasibleTime += (time(NULL) - myTime1) ;


						//////////////////////////////////////////////////////////////////////////////////////
						//Evaluating objective value for the neighboring feasible solution just created ****//
						/////////////////////////////////////////////////////////////////////////////////////

						long int myTime4=time(NULL); //Penalty calculation time

						// Penalty of new set of districts
						distPop.resize(numSubs+1);
						distDem.resize(numSubs+1);
						distPerimeter.resize(numSubs+1);
						distArea.resize(numSubs+1);


						if ( given==true){
							TPopvect[giving_district] = TPopvect[giving_district] - subPop[giving_node]; 
							TPopvect[taking_district] = TPopvect[taking_district] + subPop[giving_node];
							TDemvect[giving_district] = TDemvect[giving_district] - democrats[giving_node]; 
							TDemvect[taking_district] = TDemvect[taking_district] + democrats[giving_node];
							TRepvect[giving_district] = TRepvect[giving_district] - (voter[giving_node]-democrats[giving_node]); 
							TRepvect[taking_district] = TRepvect[taking_district] + (voter[giving_node]-democrats[giving_node]);	
							TAreavect[giving_district] = TAreavect[giving_district] - area[giving_node]; 
							TAreavect[taking_district] = TAreavect[taking_district] + area[giving_node];
							/*
							for (auto i =  Tempdistrict_wards[giving_district].begin(); i !=  Tempdistrict_wards[giving_district].end(); ++i) { 						
							if (adjacency[*i][giving_node] == 1) {
							if(std::find(accompanyingWards[giving_node].begin(), accompanyingWards[giving_node].end(), *i) == accompanyingWards[giving_node].end()){
							Mygg.push_back(*i);
							}
							}
							}
							//*/
							if (inside==1){
								TPopvect[giving_district] = TPopvect[giving_district] - subPop[giving_node_inside]; 
								TPopvect[taking_district] = TPopvect[taking_district] + subPop[giving_node_inside];
								TDemvect[giving_district] = TDemvect[giving_district] - democrats[giving_node_inside]; 
								TDemvect[taking_district] = TDemvect[taking_district] + democrats[giving_node_inside];
								TRepvect[giving_district] = TRepvect[giving_district] - (voter[giving_node_inside]-democrats[giving_node_inside]); 
								TRepvect[taking_district] = TRepvect[taking_district] + (voter[giving_node_inside]-democrats[giving_node_inside]);	
								TAreavect[giving_district] = TAreavect[giving_district] - area[giving_node_inside]; 
								TAreavect[taking_district] = TAreavect[taking_district] + area[giving_node_inside];
							}



							if (splt==1){
								/*	 
								for (auto i = giveSpltDist.begin(); i != giveSpltDist.end(); ++i) {
								cout << "giveSpltDist.begin()=" << *i << endl; 
								}
								*/
								for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
									//cout << "gg ward_districts[*k][0]=" << ward_districts[*k][0] << endl; 
									TPopvect[ward_districts[*k][0]] = TPopvect[ward_districts[*k][0]] - subPop[*k]; 
									TPopvect[taking_district] = TPopvect[taking_district] + subPop[*k];
									TDemvect[ward_districts[*k][0]] = TDemvect[ward_districts[*k][0]] - democrats[*k]; 
									TDemvect[taking_district] = TDemvect[taking_district] + democrats[*k];
									TRepvect[ward_districts[*k][0]] = TRepvect[ward_districts[*k][0]] - (voter[*k]-democrats[*k]); 
									TRepvect[taking_district] = TRepvect[taking_district] + (voter[*k]-democrats[*k]);	
									TAreavect[ward_districts[*k][0]] = TAreavect[ward_districts[*k][0]] - area[*k]; 
									TAreavect[taking_district] = TAreavect[taking_district] + area[*k];
								}
							}
						}
						else{
							TDemvect[giving_district]= TDemvect[giving_district] + democrats[taking_node]; 
							TDemvect[taking_district]= TDemvect[taking_district] - democrats[taking_node];
							TPopvect[giving_district]= TPopvect[giving_district] + subPop[taking_node]; 
							TPopvect[taking_district]= TPopvect[taking_district] - subPop[taking_node];
							TRepvect[giving_district] = TRepvect[giving_district] + (voter[taking_node]-democrats[taking_node]); 
							TRepvect[taking_district] = TRepvect[taking_district] - (voter[taking_node]-democrats[taking_node]);
							TAreavect[giving_district] = TAreavect[giving_district] + area[taking_node]; 
							TAreavect[taking_district] = TAreavect[taking_district] - area[taking_node];		
							/*
							for (auto i = Tempdistrict_wards[taking_district].begin(); i != Tempdistrict_wards[taking_district].end(); ++i) { 						
							if ( adjacency [ *i][taking_node] == 1) {
							if(std::find(accompanyingWards[taking_node].begin(), accompanyingWards[taking_node].end(), *i) == accompanyingWards[taking_node].end()){
							Mytt.push_back( *i);
							}
							}
							}
							//*/

							if (inside==2){
								TDemvect[giving_district]= TDemvect[giving_district] + democrats[taking_node_inside]; 
								TDemvect[taking_district]= TDemvect[taking_district] - democrats[taking_node_inside];
								TPopvect[giving_district]= TPopvect[giving_district] + subPop[taking_node_inside]; 
								TPopvect[taking_district]= TPopvect[taking_district] - subPop[taking_node_inside];
								TRepvect[giving_district] = TRepvect[giving_district] + (voter[taking_node_inside]-democrats[taking_node_inside]); 
								TRepvect[taking_district] = TRepvect[taking_district] - (voter[taking_node_inside]-democrats[taking_node_inside]);
								TAreavect[giving_district] = TAreavect[giving_district] + area[taking_node_inside]; 
								TAreavect[taking_district] = TAreavect[taking_district] - area[taking_node_inside];		
							}


							if (splt==2){

								for (auto k =taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
									TDemvect[giving_district]= TDemvect[giving_district] + democrats[*k]; 
									TDemvect[ward_districts[*k][0]]= TDemvect[ward_districts[*k][0]] - democrats[*k];
									TPopvect[giving_district]= TPopvect[giving_district] + subPop[*k]; 
									TPopvect[ward_districts[*k][0]]= TPopvect[ward_districts[*k][0]] - subPop[*k];
									TRepvect[giving_district] = TRepvect[giving_district] + (voter[*k]-democrats[*k]); 
									TRepvect[ward_districts[*k][0]] = TRepvect[ward_districts[*k][0]] - (voter[*k]-democrats[*k]);
									TAreavect[giving_district] = TAreavect[giving_district] + area[*k]; 
									TAreavect[ward_districts[*k][0]] = TAreavect[ward_districts[*k][0]] - area[*k];	
								}
							}
						}


						///////////////////////////
						//Population penalty


						for (auto k = districts_to_check_contiguity.begin(); k != districts_to_check_contiguity.end(); ++k) { 
							distPop[*k] = abs(TPopvect[*k]-Pbar);
						}


						// political fairness penalty based on first definition
						//distDem[giving_district] = abs(TDemvect[giving_district]-AvgNumDemocratsPerDistrict);
						//distDem[taking_district] = abs(TDemvect[taking_district]-AvgNumDemocratsPerDistrict);


						for (auto k = districts_to_check_contiguity.begin(); k != districts_to_check_contiguity.end(); ++k) { 

							distArea[*k]= TAreavect[*k];
						}

						std::sort(Mygg.begin(), Mygg.end());
						Mygg.erase(std::unique(Mygg.begin(), Mygg.end()), Mygg.end());
						std::sort(Mytt.begin(), Mytt.end());
						Mytt.erase(std::unique(Mytt.begin(), Mytt.end()), Mytt.end());
						std::sort(Mytg.begin(), Mytg.end());
						Mytg.erase(std::unique(Mytg.begin(), Mytg.end()), Mytg.end());
						std::sort(Mygt.begin(), Mygt.end());
						Mygt.erase(std::unique(Mygt.begin(), Mygt.end()), Mygt.end());

						//////////////////////////////////////////////////////////////////////////////////////
						///////////////////////Perimeter///////////////////////////////
						///////////////////////////////////
						if (given==true){
							//(1) Update perimeter of given district 
							for (auto i = Mygg.begin(); i != Mygg.end(); ++i){

								//*						
								if ( std::find(district_wards[giving_district].begin(), district_wards[giving_district].end(), *i) == district_wards[giving_district].end()) {
									continue; 
								}
								else{
									if(inside==1 && std::find(insideWards.begin(), insideWards.end(), giving_node_inside) == insideWards.end()){
										TPerivect[giving_district] += 2*(bLength[*i][giving_node]) - bLength[giving_node][giving_node_inside];
									}
									else{
										TPerivect[giving_district] += 2 * (bLength[*i][giving_node]);
									}
								}
								if (splt!=-1){
									for (auto k =giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										if ( std::find(district_wards[ward_districts[*k][0]].begin(), district_wards[ward_districts[*k][0]].end(), *i) == district_wards[ward_districts[*k][0]].end()) {
											continue; 
										}
										else{
											TPerivect[ward_districts[*k][0]] += 2* (bLength[*i][*k]);

										}

									}
									for (auto i =giving_node_split.begin(); i!= giving_node_split.end(); ++i)
										for (auto j =giving_node_split.begin(); j!= giving_node_split.end(); ++j){
											if (ward_districts[*i][0]==ward_districts[*j][0]){
												TPerivect[ward_districts[*i][0]] +=  (bLength[*i][*j]);
											}
										}
								}
								//*/
							}

							TPerivect[giving_district] -=  perimeter[giving_node];
							if (inside!=-1){
								TPerivect[giving_district] +=  -perimeter[giving_node]- perimeter[giving_node_inside];
							}

							if (splt!=-1){
								for (auto k =giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
									TPerivect[ward_districts[*k][0]] -= perimeter[*k];
								}
							}


							//	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.
							//(2) Update perimeter of taking district 
							for (auto i = Mygt.begin(); i != Mygt.end(); ++i){

								//*						
								if ( std::find(district_wards[taking_district].begin(), district_wards[taking_district].end(), *i) == district_wards[taking_district].end()) {
									continue; 
								}
								else{
									//	 if(inside==1 && std::find(insideWards.begin(), insideWards.end(), giving_node_inside) == insideWards.end()){
									//		 TPerivect[taking_district] += -2*(bLength[*i][giving_node]) - bLength[giving_node][giving_node_inside];;
									//	 }
									//	 else{
									TPerivect[taking_district] += -2*(bLength[*i][giving_node]);
									//	 }
								}
								if (splt!=-1){
									for (auto k =giving_node_split.begin(); k!= giving_node_split.end(); ++k) {

										TPerivect[taking_district] += -2*(bLength[*i][*k]);
									}

									for (auto i =giving_node_split.begin(); i!= giving_node_split.end(); ++i)
										for (auto j =giving_node_split.begin(); j!= giving_node_split.end(); ++j)
											TPerivect[taking_district] -=  (bLength[*i][*j]);
								}


								//*/
							}

							TPerivect[taking_district] +=  perimeter[giving_node];

							if (splt!=-1){
								for (auto k =giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
									TPerivect[taking_district] += perimeter[*k];
								}
							}
							if (inside!=-1){
								TPerivect[taking_district] +=  perimeter[giving_node]+ perimeter[giving_node_inside];
							}
						}
						///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						else{
							//(1) Update perimeter of taken district 
							for (auto i = Mytt.begin(); i != Mytt.end(); ++i)
							{

								//*/
								//*
								if ( std::find(district_wards[taking_district].begin(), district_wards[taking_district].end(), *i) == district_wards[taking_district].end()) {
									continue; 
								}
								else{
									if(inside==2 && std::find(insideWards.begin(), insideWards.end(), taking_node_inside) == insideWards.end()){
										TPerivect[taking_district] += 2*(bLength[*i][taking_node]) - bLength[taking_node][taking_node_inside];
									}
									else{
										TPerivect[taking_district] +=2* (bLength[*i][taking_node]);
									}
								}


								if (splt!=-1){
									for (auto k =taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										if ( std::find(district_wards[ward_districts[*k][0]].begin(), district_wards[ward_districts[*k][0]].end(), *i) == district_wards[ward_districts[*k][0]].end()) {
											continue; 
										}
										else{
											TPerivect[ward_districts[*k][0]] += 2* (bLength[*i][*k]);

										}

									}
									for (auto i =taking_node_split.begin(); i!= taking_node_split.end(); ++i)
										for (auto j =taking_node_split.begin(); j!= taking_node_split.end(); ++j){
											if (ward_districts[*i][0]==ward_districts[*j][0]){
												TPerivect[ward_districts[*i][0]] +=  (bLength[*i][*j]);
											}
										}
								}
								//*/
							}


							TPerivect[taking_district] -=  perimeter[taking_node];

							if (splt!=-1){
								for (auto k =taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
									TPerivect[ward_districts[*k][0]] -= perimeter[*k];
								}
							}
							if (inside!=-1){
								TPerivect[taking_district] +=  -perimeter[taking_node]- perimeter[taking_node_inside];
							}

							//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
							//(2) Update perimeter of given district 
							for (auto i = Mytg.begin(); i != Mytg.end(); ++i){

								//*
								if ( std::find(district_wards[giving_district].begin(), district_wards[giving_district].end(), *i) == district_wards[giving_district].end()) {
									continue; 
								}
								else{
									// if(inside==2 && std::find(insideWards.begin(), insideWards.end(), taking_node_inside) == insideWards.end()){
									//	 TPerivect[giving_district] += -2*(bLength[*i][taking_node]) - bLength[giving_node][taking_node_inside];;
									// }
									// else{
									TPerivect[giving_district] +=  -2*(bLength[*i][taking_node]);
									// }
								}

								if (splt!=-1){
									for (auto k =taking_node_split.begin(); k!= taking_node_split.end(); ++k) {

										TPerivect[giving_district] +=  -2*(bLength[*i][*k]);
									}
									for (auto i =taking_node_split.begin(); i!= taking_node_split.end(); ++i)
										for (auto j =taking_node_split.begin(); j!= taking_node_split.end(); ++j)
											TPerivect[giving_district] -=  (bLength[*i][*j]);
								}

								//*/
							}

							TPerivect[giving_district] +=  perimeter[taking_node];

							if (splt!=-1){
								for (auto k =taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
									TPerivect[giving_district] += perimeter[*k];
								}
							}
							if (inside!=-1){
								TPerivect[giving_district] +=  perimeter[taking_node]+ perimeter[taking_node_inside];
							}
						}
						////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						//Perimeter
						//distPerimeter[giving_district]= TPerivect[giving_district];
						//distPerimeter[taking_district]=TPerivect[taking_district];
						for (auto k = districts_to_check_contiguity.begin(); k != districts_to_check_contiguity.end(); ++k) { 

							distPerimeter[*k]=TPerivect[*k];
						}


						//*/
						Dem=0; 
						Rep=0; 
						for ( int i=0; i< noDistricts ; i++){
							if (TRepvect[i] > TDemvect[i]){
								Rep=Rep+1;
							}
							else{

								Dem=Dem+1; 
							}

						}
						if ((noOfGeneration%10)>8){
							cout << Dem << "  " << Rep << endl; 
						}
						int Res=0;

						//Update penalty of residency 
						for ( int i=0; i<noDistricts ;i++){
							int r=0; 
							for (auto ii=Address.begin() ; ii < Address.end() ; ii++){
								for (auto jj=district_wards[i].begin() ; jj < district_wards[i].end() ; jj++){
									if (*jj==*ii){
										r=r+1;
									}
								}
							}
							if (r>0){
								Res=Res+r-1; 
							}
						}
						//cout << "Res=" << Res<< endl; 

						// Update efficiency gap (based on second definition of political fairness)
						/* 
						for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) {

						G = ceil (0.50* (TDemvect[*i]+TRepvect[*i]));
						if (TDemvect[*i] >= G) {
						TRGapvect[*i] = TRepvect[*i] ; 
						TDGapvect[*i] = TDemvect[*i] - G;
						}
						else { 
						TDGapvect[*i] = TDemvect[*i] ; 
						TRGapvect[*i] = TRepvect[*i] - G;
						}
						sumDGap = sumFDGap + DGapvect[*i] -DGapvect[*i];
						sumRGap = sumFRGap + RGapvect[*i] -RGapvect[*i];
						}
						//*/
						/////////////////////////////////////////////////////////////////////////////////////////////////

						PenaltyP.resize(numSubs+1);			
						PenaltyV.resize(numSubs+1);
						PenaltyCom.resize(numSubs+1);
						PenaltyR.resize(numSubs+1);
						OriginalPenaltyP.resize(numSubs+1);			
						OriginalPenaltyV.resize(numSubs+1);				
						OriginalPenaltyCom.resize(numSubs+1);	
						OriginalPenaltyR.resize(numSubs+1);

						// Normalize the values using the first mean and standarsd deviation
						// multiply by 1000 to ease SA performance (just to simplify calculation)
						// Original Normalized penalty 
						//OriginalPenaltyP[giving_district] = 1000 * (((distPop[giving_district]-meanP)/SdP));
						//OriginalPenaltyP[taking_district] = 1000 * (((distPop[taking_district]-meanP)/SdP));

						for (auto k = districts_to_check_contiguity.begin(); k != districts_to_check_contiguity.end(); ++k) { 

							OriginalPenaltyP[*k] = 1000 * (((distPop[*k]-meanP)/SdP));
						}

						//Competitiveness
						//OriginalPenaltyV[giving_district] = 1000 * (((distDem[giving_district]-meanV)/SdV));
						//OriginalPenaltyV[taking_district] = 1000 * (((distDem[taking_district]-meanV)/SdV));
						//OriginalPenaltyCom[giving_district] = 1000 *((((pow((distPerimeter[giving_district]),2)/distArea[giving_district])-meanC)/SdC));
						//OriginalPenaltyCom[taking_district] = 1000 * ((((pow((distPerimeter[taking_district]),2)/distArea[taking_district])-meanC)/SdC));

						for (auto k = districts_to_check_contiguity.begin(); k != districts_to_check_contiguity.end(); ++k) { 
							OriginalPenaltyCom[*k] = 1000 * ((((pow((distPerimeter[*k]),2)/distArea[*k])-meanC)/SdC));
						}

						//Plitical Fairness penalty based on efficiency gap (second definition)				
						//beforeNormalVote = abs(sumDGap/VD-sumRGap/VR)/2;
						//Plitical Fairness penalty based on #of seats (third definition)
						beforeNormalVote=abs(Rep - ceil ((1-Vote)*noDistricts)); 
						OriginalNextDistPenaltyV=10000*(beforeNormalVote-meanV)/SdV;
						//OriginalNextDistPenaltyV=(beforeNormalVote);
						//*/

						beforeNormalRes=Res; 
						OriginalNextDistPenaltyR=1000*(beforeNormalRes-meanR)/SdR;

						//OriginalNextDistPenalty = OriginalFirstPenalty;
						OriginalNextDistPenaltyP = OriginalFirstPenaltyP; 
						//OriginalNextDistPenaltyV = OriginalFirstPenaltyV;
						OriginalNextDistPenaltyCom = OriginalFirstPenaltyCom;
						//nextDistPenalty = firstPenalty;
						//nextDistPenaltyP = firstPenaltyP;
						//nextDistPenaltyV = firstPenaltyV;
						//nextDistPenaltyCom = firstPenaltyCom;
						beforeNormalPop =beforeNormalFirstPop;
						//beforeNormalVote=beforeNormalFirstVote;
						beforeNormalCom =beforeNormalFirstCom;

						for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) { 

							// Original unweighted Normalized total penalty 
							OriginalNextDistPenaltyP += OriginalPenaltyP[*i] - OriginalFPenaltyP[*i];
							//OriginalNextDistPenaltyV += OriginalPenaltyV[*i]- OriginalFPenaltyV[*i];
							OriginalNextDistPenaltyCom += OriginalPenaltyCom[*i] - OriginalFPenaltyCom[*i];

							// Original unweighted not-Normalized total penalty 
							beforeNormalPop += distPop[*i] -distFPop[*i];
							//beforeNormalVote +=  distDem[*i]- distFDem[*i];
							beforeNormalCom +=  pow(distPerimeter[*i],2)/distArea[*i] -pow(distFPerimeter[*i],2)/distFArea[*i];
						}

						// Original Normalized weighted Total penalty 
						nextDistPenaltyP =  w1*OriginalNextDistPenaltyP;				
						nextDistPenaltyV = w2*OriginalNextDistPenaltyV;
						nextDistPenaltyCom = w3*OriginalNextDistPenaltyCom;
						nextDistPenaltyR = w4*OriginalNextDistPenaltyR;

						OriginalNextDistPenalty= OriginalNextDistPenaltyP + OriginalNextDistPenaltyV +OriginalNextDistPenaltyCom+OriginalNextDistPenaltyR;
						nextDistPenalty =  nextDistPenaltyP + nextDistPenaltyV + nextDistPenaltyCom+nextDistPenaltyR;

						Penaltycaltime += time(NULL) - myTime4; //Penalty calculation time
						//cout << nextDistPenaltyP << "," << nextDistPenaltyV << "," << nextDistPenaltyCom << endl; 
						//cout << nextDistPenalty << endl;
						//cout << "nextDistPenaltyP="<< nextDistPenaltyP << endl;
						//cout << "nextDistPenaltyV="<< nextDistPenaltyV << endl;
						//cout << "nextDistPenaltyCom="<< nextDistPenaltyCom << endl;		
						//cout << "nextDistPenaltyR="<< nextDistPenaltyR << endl;ou

						if (evaluation_output==true){						
							cout << "nextDistPenalty="<< nextDistPenalty << endl;
							cout << "nextDistPenaltyP="<< nextDistPenaltyP << endl;
							cout << "nextDistPenaltyV="<< nextDistPenaltyV << endl;
							cout << "nextDistPenaltyCom="<< nextDistPenaltyCom << endl;	
							cout << "nextDistPenaltyR="<< nextDistPenaltyR << endl;	
						}

						noOfFeasibleSolution=noOfFeasibleSolution+1;

						long int myTime5=time(NULL); // checking Possiblity (Acceptance)
						if (evaluation_output==true){
							cout << "unweighted not normalized NextDistPenaltyV=" << beforeNormalVote << endl; 
							cout << "unweighted normalized NextDistPenalty=" << OriginalNextDistPenalty << endl; 
							cout << "Weighted normalized nextDistPenalty=" << nextDistPenalty << endl; 
							cout << "firstPenalty=" << firstPenalty << endl; 
						}
						//cout << "firstPenalty=" << firstPenalty << endl; 
						double objValDifference = (nextDistPenalty-firstPenalty);
						//double objValDifference = beforeNormalVote-beforeNormalFirstVote;
						if (evaluation_output==true){
							cout << "difference = " <<objValDifference << endl;
						}
						///////////////////////////////////////////////////////////////////////////

						long int myTime5_00=time(NULL);
						if (objValDifference >= 0)
						{
							acceptanceProbability = exp((-1.00*objValDifference)/temperature);
							if(evaluation_output==true){
								/**/       cout << "\t" << "Candidate state's objective value is WORSE by "
									/**/            << objValDifference << endl;
								/**/        cout << "\t" << "The move to candidate state will be accepted with probability "
									/**/             << acceptanceProbability << endl;
							}
							double randNum=(static_cast<double>(rand()))/(RAND_MAX+1);//rand()/32672.0;
							if(evaluation_output==true){
								/**/       cout << "\t" << "Random number generated is " << randNum;
							}
							if (randNum < acceptanceProbability)
							{
								if(detailed_output==true){
									/**/       cout << "\t\t\t" << "Candidate is accepted." << endl;
								}
								accept = true;
								steps++;						
							}   
							else
							{
								if(evaluation_output==true){
									/**/         cout << "\t\t\t" << "Candidate is rejected." << endl;
								}
								accept = false;
								//cout << "false" << endl; 

								for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) { 

									TDemvect[*i]= Demvect[*i]; 

									TRepvect[*i]= Repvect[*i]; 

									TPopvect[*i]= Popvect[*i];

									TAreavect[*i] = Areavect[*i]; 

									TPerivect[*i] = Perivect[*i]; 

									Tempdistrict_wards[*i]=district_wards[*i];
								}

							}
						}
						else
						{
							if(evaluation_output==true){
								/**/       cout << "\t" << "Candidate's objective value is BETTER by " << (-1.00)*(objValDifference)
									/**/            << ", so candidate is automatically accepted." << endl;
							}
							accept = true;
							steps++;
						}

						AcceptanceTime00 += time(NULL)-myTime5_00;				
						if (accept == true)
						{
							long int myTime5_01=time(NULL);
							//cout << "true" << endl; 

							firstPenalty = nextDistPenalty;
							firstPenaltyP = nextDistPenaltyP;
							firstPenaltyV = nextDistPenaltyV;
							firstPenaltyCom = nextDistPenaltyCom;
							firstPenaltyR = nextDistPenaltyR;

							OriginalFirstPenalty = OriginalNextDistPenalty ;
							OriginalFirstPenaltyP = OriginalNextDistPenaltyP ;
							OriginalFirstPenaltyV = OriginalNextDistPenaltyV;
							OriginalFirstPenaltyCom = OriginalNextDistPenaltyCom;
							OriginalFirstPenaltyR = OriginalNextDistPenaltyR;

							beforeNormalFirstPop=beforeNormalPop ;
							beforeNormalFirstVote=beforeNormalVote;
							beforeNormalFirstRes= beforeNormalRes;


							for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) { 
								Demvect[*i]= TDemvect[*i]; 

								Repvect[*i]= TRepvect[*i]; 

								Popvect[*i]= TPopvect[*i];

								Areavect[*i] = TAreavect[*i]; 

								Perivect[*i] = TPerivect[*i]; 

								// Update the contained wards inside the given and taken districts

								district_wards[*i]=Tempdistrict_wards[*i];					

							}

							for (auto i = districts_to_check_contiguity.begin(); i != districts_to_check_contiguity.end(); ++i) { 
								distFPop[*i] = distPop[*i];
								distFDem[*i] = distDem[*i];
								distFPerimeter[*i]=distPerimeter[*i];
								distFArea[*i]= distArea[*i];
								OriginalFPenaltyP[*i]= OriginalPenaltyP[*i];
								OriginalFPenaltyV[*i]=OriginalPenaltyV[*i];
								OriginalFPenaltyCom[*i]=OriginalPenaltyCom[*i];
								OriginalFPenaltyR[*i]=OriginalPenaltyR[*i];
							}

							AcceptanceTime01 += time(NULL)-myTime5_01;			
							long int myTime5_02=time(NULL);

							vector < double >  myTempCorner;			
							//update the edges of district
							//*
							if ( given==true ) {
								//(1) add the neighbors of the given node to the corner of given district if they aren't already edges
								for (auto i=district_wards[giving_district].begin() ; i != district_wards[giving_district].end() ; i++) {
									if (adjacency [*i][giving_node]==1) {
										if(std::find(corners_nodes[giving_district].begin(), corners_nodes[giving_district].end(), *i) == corners_nodes[giving_district].end()){
											corners_nodes[giving_district].push_back(*i);
										}
									}
								}
								
								if (splt==1){
									for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										for (auto i = neighbors[*k].begin(); i!= neighbors[*k].end(); ++i) {
											if(std::find(giving_node_split.begin(), giving_node_split.end(), *i) == giving_node_split.end()){
												if (ward_districts[*k][0]==ward_districts[*i][0] && ward_districts[*k][0]!=taking_district ){
													if(std::find(corners_nodes[ward_districts[*k][0]].begin(), corners_nodes[ward_districts[*k][0]].end(), *i) == corners_nodes[ward_districts[*k][0]].end()){
														corners_nodes[ward_districts[*k][0]].push_back(*i);
													}
												}
											}
										}
									}
								}
								
								///////////////////////////////////////////////////////////////////////////////////////////////////////////
								//(2) earese given node from given district edge
								corners_nodes[giving_district].erase(std::remove(begin(corners_nodes[giving_district]), end(corners_nodes[giving_district]), giving_node), end(corners_nodes[giving_district]));

								if (splt==1){

									for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										//cout << "ward_districts[*k][0]" << ward_districts[*k][0]<< endl; 
										if(ward_districts[*k][0]!=taking_district){
											corners_nodes[ward_districts[*k][0]].erase(std::remove(begin(corners_nodes[ ward_districts[*k][0]]), end(corners_nodes[ ward_districts[*k][0]]), *k), end(corners_nodes[ ward_districts[*k][0]]));
										}
									}			
								}

								// update the assigened district to each ward
								ward_districts[giving_node][0]=taking_district; 
								if(inside==1){
									ward_districts[giving_node_inside][0]=taking_district; 
								}

								if (splt==1){

									for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										//cout << "ward_districts[*k][0] before " << ward_districts[*k][0]<< endl; 
										ward_districts[*k][0]=taking_district; 
										// cout << "ward_districts[*k][0] after " << ward_districts[*k][0]<< endl; 
									}
								}
							
							
								///////////////////////////////////////////////////////////////////////////////////////////////////////////

								//(4) add given node to taken district corner 
								// if(std::find(corners_nodes[taking_district].begin(), corners_nodes[taking_district].end(), giving_node) == corners_nodes[taking_district].end()){
								corners_nodes[taking_district].push_back(giving_node);
								// }
								if (splt==1){
									for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
										//if(std::find(corners_nodes[taking_district].begin(), corners_nodes[taking_district].end(), *k) == corners_nodes[taking_district].end()){
										if(ward_districts[*k][0]!=taking_district){
											corners_nodes[taking_district].push_back(*k);
										}
									}
								}


								///////////////////////////////////////////////////////////////////////////////////////////////////////////

								//(3)update the edges of taken distict
								myTempCorner.clear();
								for (auto i=corners_nodes[taking_district].begin() ; i != corners_nodes[taking_district].end() ; i++) {
									if (adjacency [*i][giving_node]==1) {
										//cout << "neigbor of given node" << *i << endl;
										myTempCorner.push_back(*i);
									}

									if (splt==1){
										for (auto k = giving_node_split.begin(); k!= giving_node_split.end(); ++k) {
											if (adjacency [*i][*k]==1) {
												//cout << "neigbor of given node" << *i << endl;
												myTempCorner.push_back(*i);
											}

										}
									}
								}
								sort( myTempCorner.begin(), myTempCorner.end() );
								myTempCorner.erase(std::unique(myTempCorner.begin(), myTempCorner.end()),myTempCorner.end());

								int u;
								for (auto j=myTempCorner.begin(); j!=myTempCorner.end(); j++ ){
									u=-1;
									for (int i=0 ; i < corners_nodes.size() ; i++) {
										for (int k=0 ; k < corners_nodes[i].size() ; k++) {
											if(i!= taking_district ){
												//if (corners_nodes[i][k]!=giving_node && adjacency [*j][corners_nodes[i][k]]==1){
												if (adjacency [*j][corners_nodes[i][k]]==1){
													u=0;
													break;
													// corners_nodes[taking_district].erase(corners_nodes[taking_district].begin()+ *j);
												}

											}
											else{
												break;
											}
										}
										if (u==0) {
											break;
										}
									}
									if (u==-1) {
										//	   cout << "Removed_node" <<*j << endl; 
										corners_nodes[taking_district].erase(std::remove(begin(corners_nodes[taking_district]), end(corners_nodes[taking_district]), *j), end(corners_nodes[taking_district]));
									}
								}
							
								///////////////////////////////////////////////////////////////////////////////////////////////////////////

							}

							else {
								//(1) add the neighbors of the taken node to the corner of taken district
								for (auto i=district_wards[taking_district].begin() ; i != district_wards[taking_district].end() ; i++) {
									if (adjacency [*i][taking_node]==1){
										if ( std::find(corners_nodes[taking_district].begin(), corners_nodes[taking_district].end(), *i) == corners_nodes[taking_district].end()) {
											corners_nodes[taking_district].push_back(*i);
										}
									}
								}
								
								if (splt==2){
									for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										for (auto i = neighbors[*k].begin(); i!= neighbors[*k].end(); ++i) {
											if(std::find(taking_node_split.begin(), taking_node_split.end(), *i) == taking_node_split.end()){
												if (ward_districts[*k][0]==ward_districts[*i][0] && ward_districts[*k][0]!=giving_district){
													if(std::find(corners_nodes[ward_districts[*k][0]].begin(), corners_nodes[ward_districts[*k][0]].end(), *i) == corners_nodes[ward_districts[*k][0]].end()){
														corners_nodes[ward_districts[*k][0]].push_back(*i);

													}
												}
											}
										}
									}
								}
							
								///////////////////////////////////////////////////////////////////////////////////////////////////////////
								//(2) earese taken node from taken district
								corners_nodes[taking_district].erase(std::remove(begin(corners_nodes[taking_district]), end(corners_nodes[taking_district]), taking_node), end(corners_nodes[taking_district]));

								if (splt==2){

									for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										//cout << "ward_districts[*k][0]  " << ward_districts[*k][0]<< endl; 
										if(ward_districts[*k][0]!=giving_district){
											corners_nodes[ ward_districts[*k][0]].erase(std::remove(begin(corners_nodes[ ward_districts[*k][0]]), end(corners_nodes[ ward_districts[*k][0]]), *k), end(corners_nodes[ ward_districts[*k][0]]));
										}
									}
								}


								// first update the assigened district to each ward
								ward_districts[taking_node][0]=giving_district; 
								if(inside==2){					
									ward_districts[taking_node_inside][0]=giving_district; 
								}
								if (splt==2){
									for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										// cout << "ward_districts[*k][0] before " << ward_districts[*k][0]<< endl; 
										ward_districts[*k][0]=giving_district; 
										// cout << "ward_districts[*k][0] after " << ward_districts[*k][0]<< endl; 
									}
								}


								//cout << "first " << endl; 

								
								///////////////////////////////////////////////////////////////////////////////////////////////////////////
								// (4)add taken node to given district corner
								//if(std::find(corners_nodes[giving_district].begin(), corners_nodes[giving_district].end(), taking_node) == corners_nodes[giving_district].end()){

								corners_nodes[giving_district].push_back(taking_node);
								//}
								if (splt==2){
									for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
										//if(std::find(corners_nodes[giving_district].begin(), corners_nodes[giving_district].end(), *k) == corners_nodes[giving_district].end()){	
										if(ward_districts[*k][0]!=giving_district){
											corners_nodes[giving_district].push_back(*k);
										}
									}
								}

													
								///////////////////////////////////////////////////////////////////////////////////////////////////////////

								//(3)update the edges of given distict
								myTempCorner.clear();
								for (auto i=corners_nodes[giving_district].begin() ; i != corners_nodes[giving_district].end() ; i++) {
									if ( adjacency [*i][taking_node]==1 ) {

										//	cout << "neigbor of given node" << *i << endl;
										myTempCorner.push_back(*i);
									}

									if (splt==2){
										for (auto k = taking_node_split.begin(); k!= taking_node_split.end(); ++k) {
											if (adjacency [*i][*k]==1) {
												//	cout << "neigbor of given node" << *i << endl;
												myTempCorner.push_back(*i);
											}

										}
									}
								}
								sort( myTempCorner.begin(), myTempCorner.end() );
								myTempCorner.erase(std::unique(myTempCorner.begin(), myTempCorner.end()),myTempCorner.end());
								int u;
								for (auto j=myTempCorner.begin(); j!=myTempCorner.end(); j++ ){
									u=-1;
									for (int i=0 ; i < corners_nodes.size() ; i++) {
										for (int k=0 ; k < corners_nodes[i].size() ; k++) {
											if(i!= giving_district ){

												//if (corners_nodes[i][k]!= taking_node && adjacency [*j][corners_nodes[i][k]]==1){
												if (adjacency [*j][corners_nodes[i][k]]==1){
													u=0;
													break;							
													// corners_nodes[taking_district].erase(corners_nodes[taking_district].begin()+ *j);
												}
											}
											else{
												break;
											}
										}
										if (u==0) {
											break;
										}
									}
									if (u==-1) {
										// cout << "Removed_node" <<*j << endl; 
										corners_nodes[giving_district].erase(std::remove(begin(corners_nodes[giving_district]), end(corners_nodes[giving_district]), *j), end(corners_nodes[giving_district]));
									}
								}
							
							}

							//*/

							//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							if(edges_output==true){
								for (auto ii = districts_to_check_contiguity.begin(); ii != districts_to_check_contiguity.end(); ++ii) { 
									cout << "edges of district" << *ii << endl; 
									for (int j = 0; j < corners_nodes[*ii].size(); j++) {     
										cout << corners_nodes[*ii][j] << ",";
									}
									cout << endl; 
								}
							}
							AcceptanceTime02 += time(NULL)-myTime5_02;
							long int myTime5_03=time(NULL);
							HighPopNodes.clear();
							HighPop.clear();
							LowPopNodes.clear();


							for (int i = 0; i <noDistricts ; i++){	
								if (abs(Popvect[i]-Pbar)/Pbar >= populationDeviation){
									HighPopNodes.push_back(i);
									HighPop.push_back(abs(Popvect[i]-Pbar)/Pbar);
								}						
							}
							//cout << "listsizeHP---------" << listsizeHP << endl;
							listsizeHP = HighPopNodes.size();
							if(detailed_output==true){
								cout << "listsizeHP---------" << listsizeHP << endl;
							}
							AcceptanceTime03 += time(NULL)-myTime5_03;
						}
						iterations++;

						//Compare the current set of districts (after "iterations" iterations and "steps" steps are taken)
						//to the best set.  If current districts better, set best district = current districts.
						//if (firstPenalty < bestDistPenalty)
						//if (firstPenaltyV < bestDistPenaltyV)
						if ((listsizeHP==0) || (listsizeHP!=0 && firstPenalty < bestDistPenalty ) )
						{
							bestIteration = iterations;
							bestStep = steps;

							best_district_wards.clear(); 
							best_district_wards.resize(noDistricts); 
							for (int i=0 ; i < district_wards.size(); i++){
								for (int j=0 ; j < district_wards[i].size(); j++){
									best_district_wards[i].push_back(district_wards[i][j]);
								}
							}

							bestDistPenalty = firstPenalty;
							bestDistPenaltyP = firstPenaltyP;
							bestDistPenaltyV = firstPenaltyV;
							bestDistPenaltyCom = firstPenaltyCom;
							bestDistPenaltyR = firstPenaltyR;

							OriginalBestDistPenalty = OriginalFirstPenalty;
							OriginalBestDistPenaltyP = OriginalFirstPenaltyP;
							OriginalBestDistPenaltyV = OriginalFirstPenaltyV;
							OriginalBestDistPenaltyCom = OriginalFirstPenaltyCom;
							OriginalBestDistPenaltyR = OriginalFirstPenaltyR;

							bestBeforeNormalPenalty = beforeNormalFirstPop + beforeNormalFirstVote+ beforeNormalFirstCom+beforeNormalFirstRes;
							bestBeforeNormalP = beforeNormalFirstPop;
							bestBeforeNormalV = beforeNormalFirstVote;
							bestBeforeNormalCom = beforeNormalFirstCom;
							bestBeforeNormalR = beforeNormalFirstRes;

							bestTime = time(NULL) - startTime;
						}

						AcceptanceTime += time(NULL)-myTime5;
						runTime = time(NULL) - startTime;
						if(evaluation_output==true){
							cout << "temperature=" << temperature << endl; 
							cout << "steps=" << steps << endl; 
						}

						if (accept==true && listsizeHP==0){
							break;
							//w1 = 0;
							//w2 = 1; 
							//w3 = 1;
							//firstPenalty= firstPenaltyStore/100;

						}
					}  //while (iterations < NUM_ITERATIONS)

					iterations=0;
					/*
					////////Reheat//////////
					if (xx<=10)
					{
					if (steps<= 70000)
					{
					temperature*=TEMP_FACTOR;
					}
					else
					{
					//temperature= 0.3*START_TEMPERATURE;
					temperature=10;
					steps=30000;
					int xx=xx+1;
					}
					}
					else
					{
					//*/
					temperature*=TEMP_FACTOR;
					//}
					if (accept == true&& listsizeHP==0){
						break;
					}

				}  //while (runTime <  .... )

				//.................
				//} //while (temperature > FinalTemp)

				///////////////////////////////////////
				// Print result***************////////
				/////////////////////////////////////
				cout << endl;
				cout << "\t" << "The best score was achieved after iteration " << bestIteration
					<< " after " << bestStep << " steps." << endl;
				cout << "\t" << "Objective value of best state =    " << bestDistPenalty << endl;		

				myOutputFile << "-------------------------------------------------------------" << endl; 
				myOutputFile << "Population:" << endl; 
				double VoteP=0;
				double VoteP1=0;
				double VoteP2=0;

				Rep=0;
				Dem=0;
				for (int i=0 ; i < best_district_wards.size(); i++){
					double P=0;
					double S=0;
					double GapR=0;
					double GapD=0;
					double Temp=0;
					double G=0; 
					double R=0; 
					double D=0; 
					double B=0; 
					double H=0; 
					for (int j=0 ; j < best_district_wards[i].size(); j++){
						P +=subPop[best_district_wards[i][j]];
						D += democrats[best_district_wards[i][j]];
						B +=black [best_district_wards[i][j]];
						H +=hispanic[best_district_wards[i][j]]; 
						R += voter[best_district_wards[i][j]]-democrats[best_district_wards[i][j]];
						S += area[best_district_wards[i][j]];
						for (int k=0; k<numSubs; k++)
						{
							if(std::find(best_district_wards[i].begin(), best_district_wards[i].end(), k) == best_district_wards[i].end())  
							{   
								if(adjacency[best_district_wards[i][j]][k]==1){
									Temp = Temp+bLength[best_district_wards[i][j]][k];                                  
								}
							}
						}
					}

					// Efficiency Gap
					//*
					G = ceil (0.50* (R+D));
					if (D >= G) {
						GapR = R ; 
						GapD = D - G;
					}
					else { 
						GapD = D ; 
						GapR = R - G;
					}
					//*/
					if(detailed_output==true){
						cout << "district[" << i << "]: PopDeviation=" << abs(P-Pbar)/ Pbar << " and populstion= " << P << " and ratio of democrats= " << D/(R+D) << " and democrats's gap= " << GapD <<" and Republicans' gap=" << GapR << " and compactness=" << (pow((Temp),2)/S) << endl; 
					}
					

					//myOutputFile << "Population:" << endl; 
					//myOutputFile << "district[" << i << "]: PopDeviation=" << abs(P-Pbar)/ Pbar << " and populstion= " << P << " and ratio of democrats= " << D/(R+D) << " and democrats's gap= " << GapD <<" and Republicans' gap=" << GapR << " and compactness=" << (pow((Temp),2)/S) << endl; 
					//myOutputFile << "balck and Hispanic pop:"  << endl; 
					//myOutputFile << i  << " = " <<   P  << " and " << B  << " and " << H << " and " << B/P << " and " << H/P << endl;
					//myOutputFile << "Democrats and Republicans and compactness:"  << endl; 
					//myOutputFile << P << " and " << D << " and " << D+R << " and " << S << " and " << Temp << " and " << D-R << endl; 
					myOutputFile << i  << " = " <<  P  << " , " << abs(P-Pbar)/ Pbar << " , " << D << " , " << R << " , " << D+R << " , " <<  D/(R+D) << " , " << B  << " , " << H << " , " << (pow((Temp),2)/S)  << endl;
					
					
					if (R > D){
						Rep=Rep+1;
					}
					else{
						Dem=Dem+1;  
					}
					VoteP1 +=(GapD);
					VoteP2 += (GapR);
				};
				VoteP = ((VoteP1)-(VoteP2));

				myOutputFile << "-------------------------------------------------------------" << endl; 

				if ( VoteP1/VD >= VoteP2/VR ) { 
					myOutputFile << " Republicans have done better job" << endl; 
				}
				else {
					myOutputFile << " Democrats have done better job" << endl; 
				}
				myOutputFile << "newGap=" << abs(VoteP1/VD-VoteP2/VR)/2 << endl;
				myOutputFile << "OldGap=" <<  abs(VoteP/TVoter) << " -- "  << abs(VoteP/TVoter + 0.5 * (VR-VD)/TVoter) << endl;

				//myOutputFile << "MaxBoundOfVoteRatio" << 2*(VD -VoteP1)/TVoter << " and " << 2*(VR -VoteP2)/TVoter << endl;
				myOutputFile << "#OfRepSeats=" << Rep << endl; 
				myOutputFile << "#OfDemSeats=" << Dem << endl;
				//*/
				cout << "\nTotal seconds elapsed = " << time(NULL) - startTime << endl;
				// output file 
				//*
				myOutputFile << endl;
				myOutputFile << "-------------------------------------------------------------" << endl; 
				myOutputFile << "\t" << "The best score was achieved after iteration= " << bestIteration
					<< " after " << bestStep << " steps." << endl;
				myOutputFile << "\t" << "Objective value of best state =    " << bestDistPenalty << endl;
				myOutputFile << "\t" << "Objective value of weighted normalized  pop, vot , com and Res =    " << bestDistPenaltyP << "," <<bestDistPenaltyV <<  ","  <<  bestDistPenaltyCom <<  bestDistPenaltyR <<endl;
				myOutputFile << "\t" << "Objective value of best Original state =    " << OriginalBestDistPenalty << endl;
				myOutputFile << "\t" << "Objective value of not weighted but normalized pop, vote ,com, and Res=    " <<  OriginalBestDistPenaltyP << "," << OriginalBestDistPenaltyV <<  ","  <<   OriginalBestDistPenaltyCom << "," << OriginalBestDistPenaltyR <<endl;
				myOutputFile << "\t" << "Objective value of not weighted not normalized pop, vote, com, and Res=    " <<  bestBeforeNormalP << "," << bestBeforeNormalV <<  ","  <<   bestBeforeNormalCom << "," << bestBeforeNormalR <<endl;

				myOutputFile << "-------------------------------------------------------------" << endl; 
				myOutputFile << "noOfNeighbors=" << noOfGeneration << endl;
				myOutputFile << "noOfFeasibleSolution=" << noOfFeasibleSolution << endl;
				myOutputFile << "\nBestTime = " << bestTime << endl;
				myOutputFile <<"\nneighbortime = "<< neighbortime<< endl;
				//myOutputFile <<"\nneighbortime00 = "<< neighbortime00 << endl;
				//myOutputFile <<"\nneighbortime01 = "<< neighbortime01 << endl;
				//myOutputFile <<"\nneighbortime02 = "<< neighbortime02 << endl;
				//myOutputFile <<"\nneighbortime03 = "<< neighbortime03 << endl;
				myOutputFile <<"\ncontiguityTime = "<< contiguityTime << endl;
				//myOutputFile <<"\ncontiguityTime00 = "<< contiguityTime00 << endl;
				//myOutputFile <<"\ncontiguityTime01 = "<< contiguityTime01 << endl;
				//myOutputFile <<"\ncontiguityTime02 = "<< contiguityTime02 << endl;
				myOutputFile <<"\nfeasibleTime = "<< feasibleTime << endl;
				myOutputFile <<"\nPenaltycaltime "<< Penaltycaltime << endl;
				myOutputFile << "\nAcceptanceTime = "<< AcceptanceTime << endl;
				//myOutputFile << "\nAcceptanceTime00 = "<< AcceptanceTime00 << endl;
				//myOutputFile << "\nAcceptanceTime01 = "<< AcceptanceTime01 << endl;
				//myOutputFile << "\nAcceptanceTime02 = "<< AcceptanceTime02 << endl;
				//myOutputFile << "\nAcceptanceTime03 = "<< AcceptanceTime03 << endl;
				myOutputFile << "\ninitialtime = "<< initialtime << endl;	
				myOutputFile << "\nTotal seconds elapsed = " << time(NULL) - startTime << endl;		
				myOutputFile << "\t" << "The best districts found was: ";
				/*
				for (int i=0 ; i < best_district_wards.size(); i++){
				for (int j=0 ; j < best_district_wards[i].size(); j++){
				myOutputFile <<best_district_wards[i][j] << ",";
				}
				}
				//*/
				////////////////////////////////////////////////////
				//*
				/*
				myOutputFile3 <<" size()=" << myNotContiguousWards.size() << endl; 
				for (int i=0 ; i < myNotContiguousWards.size(); i++){
				myOutputFile3 << myNotContiguousWards[i] << "," << endl ; 
				}
				//*/
				myOutputFile << "-------------------------------------------------------------" << endl; 
				cout << "Details of assignment: " << endl; 
				myOutputFile << "Details of assignment: " << endl;
				for (int i=0 ; i < best_district_wards.size(); i++){
					for (int j=0 ; j < best_district_wards[i].size(); j++){
						//cout << "ward["<< best_district_wards[i][j] << "]is assigned to district[" << i <<"];" <<endl; 
						//myOutputFile  <<  best_district_wards[i][j] << "       " << i  <<endl;	
						myOutputFile << i << "         "  <<  best_district_wards[i][j] << endl;
					}
				}
				//*/

							//*/
				cout << "Details of address assignment: " << endl; 
				myOutputFile << "-------------------------------------------------------------" << endl; 
				myOutputFile << "Details of address assignment: " << endl; 

				for (int i=0 ; i < best_district_wards.size(); i++){
					int sa=0;
					int ra=0;
					for (int j=0 ; j < best_district_wards[i].size(); j++){
						//for (auto k = SenAddress.begin(); k!= SenAddress.end(); ++k) {
							if(std::find(SenAddress.begin(), SenAddress.end(), best_district_wards[i][j]) == SenAddress.end()) {
								continue;
							}
							else{
								sa=sa+1;
							}
					}
					for (int j=0 ; j < best_district_wards[i].size(); j++){
							if(std::find(Address.begin(), Address.end(), best_district_wards[i][j]) == Address.end()) {
								continue;
							}
							else{
								ra=ra+1;
							}
					}
					myOutputFile  <<  i   << "       " << ra  << "       " <<   sa   << endl;	
				}
				//*/



				// list of the neighbor districts for the best solution
				myOutputFile << "-------------------------------------------------------------" << endl; 
				myOutputFile << "list of the neighbor districts: " << endl; 
				neighbor_districts .resize(noDistricts); 
				for (int i = 0; i < noDistricts ; i++) { 
					int y=-1; 
					for (int k=0 ;  k < noDistricts ; k++) {
						if (k==i) {
							continue;
						}
						for (int j = 0; j < corners_nodes[i].size(); j++) { 
							for (int l = 0; l < corners_nodes[k].size(); l++) { 
								//cout << corners_nodes[k].size() << endl; 
								//cout << i <<" " << k  << "  "<< j << " " << l << endl;
								if (adjacency [corners_nodes[i][j]][corners_nodes[k][l]] == 1) {
									if(std::find(neighbor_districts[i].begin(), neighbor_districts[i].end(), k) == neighbor_districts[i].end()) {
										neighbor_districts[i].push_back(k); 
									}

								}
							}														}
					}
				}



				for (int i=0 ; i < neighbor_districts.size(); i++){
					for (int j=0 ; j < neighbor_districts[i].size(); j++){
						//cout << "ward["<< best_district_wards[i][j] << "]is assigned to district[" << i <<"];" <<endl; 
						myOutputFile   << i  <<  "       " << neighbor_districts[i][j]  << endl;				
					}
				}


				myOutputFile.close();
				//myOutputFile1.close();
				//myOutputFile2.close();
				//myOutputFile3.close();
				//*/
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////    
	}
	catch (IloException& ex) 
	{
		cerr << "Error: " << ex << endl;
	}
	catch (...) 
	{
		cerr << "Error" << endl;
	}
	env.end();
	return 0;
}
//*
/////////////////////////////////////////////////////////////////////////////////////
static void readData1 (const char* filename, IloInt& noDistricts, IloNumArray& subPop)
{
	ifstream in(filename);
	if (in) 
	{
		in >> noDistricts;
		in >> subPop;
		if(detailed_output==true){	
			cout << noDistricts <<endl; 
			cout << subPop <<endl;
		}

	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
static void readData2 (const char* filename, IloNumArray& democrats)
{
	ifstream in(filename);
	if (in) 
	{
		in >> democrats;

		if(detailed_output==true){	
			cout << democrats <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
static void readData3 (const char* filename, IloNumArray& republican)
{
	ifstream in(filename);
	if (in) 
	{
		in >> republican;

		if(detailed_output==true){	
			cout << republican <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
static void readData4 (const char* filename, IloNumArray& area)
{
	ifstream in(filename);
	if (in) 
	{
		in >> area;

		if(detailed_output==true){	
			cout << area <<endl;

		}
	}

	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
/////////////////////////////////////////////////////////////////////////////
static void readData5 (const char* filename, IloNumArray& perimeter)
{
	ifstream in(filename);
	if (in) 
	{
		in >> perimeter;

		if(detailed_output==true){	
			cout << perimeter <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
//////////////////////////////////////////////////////////////////////////////////
static void readData6 (const char* filename, IloNumArray& voter)
{
	ifstream in(filename);
	if (in) 
	{
		in >> voter;

		if(detailed_output==true){
			cout << voter <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
//////////////////////////////////////////////////////////////////////////////////////
static void readData7 (const char* filename, IloNumArray& amount)
{
	ifstream in(filename);
	if (in) 
	{
		in >> amount;

		if(detailed_output==true){
			cout << amount <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
static void readData8 (const char* filename, IloNumArray2& bLength)
{
	ifstream in(filename);
	if (in) 
	{
		in >> bLength;

		if(detailed_output==true){
			//cout << bLength <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void readData9 (const char* filename, IloNumArray2& adjacency)
{
	ifstream in(filename);
	if (in) 
	{
		in >> adjacency;

		if(detailed_output==true){
			//cout << adjacency <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
////////////////////////////////////////////////////////////////

static void readData10 (const char* filename, IloNumArray& black)
{
	ifstream in(filename);
	if (in) 
	{
		in >> black;

		if(detailed_output==true){
			//cout << adjacency <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
////////////////////////////////////////////////////////////////

static void readData11 (const char* filename, IloNumArray& hispanic)
{
	ifstream in(filename);
	if (in) 
	{
		in >> hispanic;

		if(detailed_output==true){
			cout << hispanic <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
////////////////////////////////////////////////////////////////
static void readData12 (const char* filename, IloNumArray& address)
{
	ifstream in(filename);
	if (in) 
	{
		in >> address;

		if(detailed_output==true){
			cout << address <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}
////////////////////////////////////////////////////////////////

static void readData13 (const char* filename, IloNumArray& senAddress)
{
	ifstream in(filename);
	if (in) 
	{
		in >> senAddress;

		if(detailed_output==true){
			cout << senAddress <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}


////////////////////////////////////////////////////////////////////

static void readData14 (const char* filename, IloNumArray2& split)
{
	ifstream in(filename);
	if (in) 
	{
		in >> split;

		if(detailed_output==true){
			//cout << split <<endl;
		}
	}
	else
	{
		cerr << "No such file: " << filename << endl;
		throw(1);
	}
}


////////////////////////////////////////////////////////////////
// Contiguity output
int traverseSubs(int node, IloNumArray2 boundry)
{
	if(contiguety_output==true){
		cout << "traverseSubs with node " << node << endl; 
	}
	// cout << "traverseSubs with node " << node << endl; 
	traversed_nodes.push_back(node);
	if(contiguety_output==true){
		cout << "Current traversed_nodes: " ;
		for (auto i = traversed_nodes.begin(); i != traversed_nodes.end(); ++i) 
		{
			std::cout << *i << ' ';
		}
		cout << endl;
	}
	for (auto i = subs_in_district.begin(); i != subs_in_district.end(); ++i) {     
		// check if initial "node" and "sub i" are neighbors
		if (boundry[node][*i] == 1) {
			// check if sub i is not already traversed
			if(std::find(traversed_nodes.begin(), traversed_nodes.end(), *i) == traversed_nodes.end()) 
			{        
				// not traversed, so add it to the list of traversed nodes and
				// call traverseSubs recursively with sub i
				traverseSubs( *i, boundry );
				if(contiguety_output==true){
					cout << *i << " added to traversed_nodes" << endl;
				}
			} 
			else
			{     
				if(contiguety_output==true){
					cout << *i << " already exists in traversed_nodes" << endl;
				}
			}
		}
	}
	return 0;
}
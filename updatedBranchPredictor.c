/*
 *  A Branch Predictor Simulator by Blake Oakley
 *
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <ctype.h>

void santize(char line[]);
float roundToTwoPlaces(float num);
int readLine(char tmp[], int sz, FILE *fp);
unsigned long createMask(int bits);

int main(int argc, char *argv[])
{
    char buff[100];//buffer to hold line from tracefile
    unsigned long addr; //the address
    char actual; //'t' for taken or 'n' for not taken
    char predict; //what we think will happen: 't' or 'n'
    double hits = 0;
    double misses = 0;
    char *branch;

    if (strcmp(argv[1], "smith") == 0)
    {
        //do smith: sim smith <B> <tracefile>
        //printf(":::smith:::\n");
        int b = atoi(argv[2]); // # of bits used in counter
        int initialized = (int) (pow(2, b)/2); // number that determines hit or miss
        int counter =  initialized; //properly initialize counter
        //printf("Counter init to : %d\n", counter);
        int max = (int)(pow(2, b) - 1);
        //printf("The max number at saturation is: %d\n", max);
        char *traceString = argv[3];
        FILE *trace = fopen(argv[3], "r");
        int total = 0;
        while(!feof(trace))
        {
            if (readLine(buff, sizeof(buff), trace))
            {
                //fgets(buff, sizeof(buff), trace); //reads line from tracefile
                //santize(buff);
                addr = strtoul(&buff[0], &branch, 16); //Gets Address and Branch
                actual = branch[1];//assigns actual 't' or 'n'
                //make prediction
                if(counter >= initialized)
                {
                    predict = 't';
                }
                else
                {
                    predict = 'n';
                }
                //increment hits or misses
                if(predict == actual)
                {
                    hits++;
                }
                else
                {
                    misses++;
                }
                //update counter
                if(actual == 't' && counter < max)
                {
                    counter++;
                }
                else if(actual == 'n' && counter > 0)
                {
                    counter--;
                }

                total++;
            }
        }
        //printf("total is : %d\n", total);
        printf("COMMAND\n./sim smith %d %s\n", b, traceString);
        printf("OUTPUT\nnumber of predictions: %.0f\nnumber of mispredictions: %.0f\nmisprediction rate: %2.2f%%\n", misses + hits, misses, 100 * (misses /(misses + hits)));
        printf("FINAL COUNTER CONTENT: %d\n", counter);

    }
    else if (strcmp(argv[1], "bimodal") == 0)
    {
        //do bimodal: sim bimodal <M2> <tracefile>
        //printf(":::bimodal:::");
        int m2 = atoi(argv[2]); // # of low-order PC bits used to form prediction table
        int tableSize = pow(2,m2);
        int initialized = 4;
        int max = 7;
        int *bimodalTable = (int *)calloc(tableSize, sizeof(int));//the table
        for(int i=0; i<tableSize; i++) //initialize every counter in table to 4
        {
            bimodalTable[i] = initialized;
        }
        unsigned long mask, index;
        char *traceString = argv[3];
        FILE *trace = fopen(argv[3], "r");
        int total = 0;

        while(!feof(trace))
        {
            if (readLine(buff, sizeof(buff), trace))
            {
                //fgets(buff, sizeof(buff), trace); //reads line from tracefile
                //santize(buff);
                addr = strtoul(&buff[0], &branch, 16); //Gets Address and Branch
                addr >>= 2;//removes offset
                mask = (1 << m2) - 1;//gets M bits from addr
                addr &= mask;
                index = addr;
                actual = branch[1];//assigns actual 't' or 'n'
                //make prediction
                if(bimodalTable[index] >= initialized)
                {
                    predict = 't';
                }
                else
                {
                    predict = 'n';
                }
                //increment hits or misses
                if(predict == actual)
                {
                    hits++;
                }
                else
                {
                    misses++;
                }
                //update counter at appropriate index in table
                if(actual == 't' && bimodalTable[index] < max)
                {
                    bimodalTable[index]++;
                }
                else if(actual == 'n' && bimodalTable[index] > 0)
                {
                    bimodalTable[index]--;
                }

                total++;
            }
        }
        //printf("total is : %d\n", total);
        printf("COMMAND\n./sim bimodal %d %s\n", m2, traceString);
        printf("OUTPUT\nnumber of predictions: %.0f\nnumber of mispredictions: %.0f\nmisprediction rate: %.2f%%\n",
               misses + hits, misses, 100 * (misses /(misses + hits)));
        printf("FINAL BIMODAL CONTENTS \n");
        for (int i = 0; i < tableSize; i++)
        {
            printf("%d  %d\n", i, bimodalTable[i]);
        }

    }
    else if (strcmp(argv[1], "gshare") == 0)
    {
        //do gshare: sim gshare <M1> <N> <tracefile>
        //printf(":::gshare:::");
        int m1 = atoi(argv[2]); // # of low-order PC bits used to form prediction table
        int n = atoi(argv[3]); //# of bits in gbhr
        unsigned long gbhr = 0; //Global Branch History Register init to zero
        int tableSize = pow(2,m1);
        int initialized = 4;
        int max = 7;
        int *gshareTable = (int *)calloc(tableSize, sizeof(int));//the table
        for(int i=0; i<tableSize; i++) //initialize every counter in table to 4
        {
            gshareTable[i] = initialized;
        }
        unsigned long index;
        unsigned long mask;
        char *traceString = argv[4];
        FILE *trace = fopen(argv[4], "r");
        int total = 0;

        while(!feof(trace))
        {
            if (readLine(buff, sizeof(buff), trace))
            {
                //fgets(buff, sizeof(buff), trace); //reads line from tracefile
                //santize(buff);
                addr = strtoul(&buff[0], &branch, 16); //Gets Address and Branch
                addr >>= 2; //removes offset
                mask = (1 << m1) - 1; //gets M bits from addr
                //mask = createMask(m1);
                addr &= mask;
                index = gbhr ^ addr; //XOR over M bits of the addr

                actual = branch[1]; //assigns actual 't' or 'n'
                //make prediction
                if(gshareTable[index] >= initialized)
                {
                    predict = 't';
                }
                else
                {
                    predict = 'n';
                }
                //increment hits or misses
                if(predict == actual)
                {
                    hits++;
                }
                else
                {
                    misses++;
                }
                //update counter at appropriate index in table
                if(actual == 't')
                {
                    if (gshareTable[index] < max)
                    {
                        gshareTable[index]++;
                    }
                    //shift 1 into BHR
                    gbhr >>= 1;
                    gbhr |= (1 << (n - 1));
                    mask = (1 << n) - 1;
                    gbhr &= mask;
                }
                else if(actual == 'n')
                {
                    if (gshareTable[index] > 0)
                    {
                        gshareTable[index]--;
                    }
                    //shift 0 into GBHR
                    gbhr >>= 1;
                    mask = (1 << n) - 1;
                    gbhr &= mask;
                }

                total++;
            }
        }
        //printf("total is : %d\n", total);
        printf("COMMAND\n./sim gshare %d %d %s\n", m1, n, traceString);
        printf("OUTPUT\nnumber of predictions: %.0f\nnumber of mispredictions: %.0f\nmisprediction rate: %.2f%%\n",
               misses + hits, misses, 100 * (misses /(misses + hits)));
        printf("FINAL GSHARE CONTENTS \n");
        for (int i = 0; i < tableSize; i++)
        {
            printf("%d  %d\n", i, gshareTable[i]);
        }
    }
    else if (strcmp(argv[1], "hybrid") == 0)
    {
        //do hybrid: sim hybrid <K> <M1> <N> <M2> <tracefile>
        //printf(":::hybrid:::\n");
        //Set-up Chooser
        int k = atoi(argv[2]); // 2^k is chooser table size
        int chooserTableSize = pow(2,k);
        int chooserInitialized = 1; // All values in Chooser Table will be initialed to 1
        int chooserMax = 3; // Max for the two bit counters in each spot in the Chooser Table
        int *chooserTable = (int *)calloc(chooserTableSize, sizeof(int));//the Chooser table
        for(int i=0; i<chooserTableSize; i++) //initialize every counter in Chooser table to 1
        {
            chooserTable[i] = chooserInitialized;
        }
        char *traceString = argv[6];
        FILE *trace = fopen(argv[6], "r");
        int total = 0;
        //Set-up BiModal
        int m2 = atoi(argv[5]); // # of low-order PC bits used to form prediction table
        int BtableSize = pow(2, m2);
        int Binitialized = 4;
        int Bmax = 7;
        int *bimodalTable = (int *)calloc(BtableSize, sizeof(int));//the table
        for(int i=0; i<BtableSize; i++) //initialize every counter in table to 4
        {
            bimodalTable[i] = Binitialized;
        }
        unsigned long Bmask, Bindex;
        //Set-up GShare
        int m1 = atoi(argv[3]); // # of low-order PC bits used to form prediction table
        int n = atoi(argv[4]); //# of bits in gbhr
        unsigned long gbhr = 0; //Global Branch History Register init to zero
        int GtableSize = pow(2,m1);
        int Ginitialized = 4;
        int Gmax = 7;
        int *gshareTable = (int *)calloc(GtableSize, sizeof(int));//the table
        for(int i=0; i<GtableSize; i++) //initialize every counter in table to 4
        {
            gshareTable[i] = Ginitialized;
        }
        unsigned long Gindex, Gmask;

        //Get Predictions from GShare and Bimodal
        char Gpredict, Bpredict;
        unsigned long Gaddr, Baddr, Hindex;

        while(!feof(trace)) //BEGIN HYBRID LOOP
        {
            if (readLine(buff, sizeof(buff), trace))
            {
                addr = strtoul(&buff[0], &branch, 16); //Gets Address and Branch
                addr >>= 2; //removes offset
                //Hindex
                Hindex = addr & ((1 << k) - 1);
                //Bimodal
                Bmask = (1 << m2) - 1;//gets M bits from addr
                Baddr = addr & Bmask;
                Bindex = Baddr;
                //Gshare
                Gmask = (1 << m1) - 1; //gets M bits from addr
                Gaddr = addr & Gmask;
                Gindex = gbhr ^ Gaddr; //XOR over M bits of the addr

                actual = branch[1]; //assigns actual 't' or 'n'
                //make Bimodal prediction
                if(bimodalTable[Bindex] >= Binitialized)
                {
                    Bpredict = 't';
                }
                else
                {
                    Bpredict = 'n';
                }
                //make Gshare prediction
                if(gshareTable[Gindex] >= Ginitialized)
                {
                    Gpredict = 't';
                }
                else
                {
                    Gpredict = 'n';
                }
                //make overall prediction
                if(chooserTable[Hindex] > chooserInitialized)
                {
                    //use gshare
                    if(Gpredict == actual)
                    {
                        hits++;
                    }
                    else
                    {
                        misses++;
                    }
                    //Update counter in GShare Table
                    if(actual == 't' && gshareTable[Gindex] < Gmax)
                    {
                        gshareTable[Gindex]++;
                    }
                    else if(actual == 'n' && gshareTable[Gindex] > 0)
                    {
                        gshareTable[Gindex]--;
                    }

                }
                else
                {
                    //use bimodal
                    if(Bpredict == actual)
                    {
                        hits++;
                    }
                    else
                    {
                        misses++;
                    }
                    //Update counter in Bimodal Table
                    if(actual == 't' && bimodalTable[Bindex] < Bmax)
                    {
                        bimodalTable[Bindex]++;
                    }
                    else if(actual == 'n' && bimodalTable[Bindex] > 0)
                    {
                        bimodalTable[Bindex]--;
                    }
                }
                //Update GBHR regardless of which method was chosen
                if(actual == 't')
                {

                    //shift 1 into BHR
                    gbhr >>= 1;
                    gbhr |= (1 << (n - 1));
                    Gmask = (1 << n) - 1;
                    gbhr &= Gmask;
                }
                else if(actual == 'n')
                {

                    //shift 0 into GBHR
                    gbhr >>= 1;
                    Gmask = (1 << n) - 1;
                    gbhr &= Gmask;
                }
                //Update the counter at the appropriate index in the Chooser Table
                if((actual == Gpredict && actual == Bpredict) || (actual != Gpredict && actual != Bpredict))
                {
                    //no change
                }
                else if ((actual == Gpredict && actual != Bpredict) && chooserTable[Hindex] < chooserMax)
                {
                    chooserTable[Hindex]++;
                }
                else if ((actual != Gpredict && actual == Bpredict) && chooserTable[Hindex] > 0)
                {
                    chooserTable[Hindex]--;
                }

                total++;
            }
        }
        //END HYBRID LOOP
        //printf("total is : %d\n", total);
        printf("COMMAND\n./sim hybrid %d %d %d %d %s\n", k, m1, n, m2, traceString);
        printf("OUTPUT\nnumber of predictions: %.0f\nnumber of mispredictions: %.0f\nmisprediction rate: %.2f%%\n",
               misses + hits, misses, 100 * (misses /(misses + hits)));
        printf("FINAL CHOOSER CONTENTS \n");
        for (int i = 0; i < chooserTableSize; i++)
        {
            printf("%d  %d\n", i, chooserTable[i]);
        }
        printf("FINAL GSHARE CONTENTS \n");
        for (int i = 0; i < GtableSize; i++)
        {
            printf("%d  %d\n", i, gshareTable[i]);
        }
        printf("FINAL BIMODAL CONTENTS \n");
        for (int i = 0; i < BtableSize; i++)
        {
            printf("%d  %d\n", i, bimodalTable[i]);
        }

    }
//    else
//    {
//        //error
//        printf("error");
//    }


    return 0;
}

void santize(char line[])
{
	int offset = 0;
	while (isdigit((line[offset])) && offset < strlen(line))
	{
		offset++;
	}

	if (offset != 0)
	{
		memcpy(line, &line[offset], strlen(line));
	}
}
float roundToTwoPlaces(float num)
{
    float finished = (int)(num * 100 + .5);
    return (float)finished / 100;
}
int readLine(char tmp[], int sz, FILE *fp)
{
    if (fgets(tmp, sz, fp) != NULL)
    {
        strtok(tmp, "\n");
        if (strlen(tmp) > 0 && strcmp(tmp, "\n") != 0)
        {
            return 1;
        }
    }
    return 0;
}
unsigned long createMask(int bits)
{
	unsigned long ret = 0;

	for (int i = 0; i < bits; i++)
	{
		ret = (ret << 1) | 1;
	}

	return ret;
}

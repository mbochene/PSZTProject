#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <math.h>
#include <random>
#include "neuron.hpp"

using namespace std;

class mlp{
    /* wektor przechowujący informację o liczbie wejść do neuronów w każdej z warstw;
     * elementy wektora o indeksach od 1 wzwyż można też traktować jako
     * liczbę neuronów w danej warstwie sieci 
     * */ 
    vector<int> layerInputs;

    vector<vector<neuron>> neurons;         // wektor wektorów reprezentujący neurony i ich ułożenie w sieci
    vector<double> networkOutput;           // wektor przechowujący wyjścia sieci
    vector<double> expectedNetworkOutput;   // wektor przechowujący oczekiwane wyjścia sieci (proces uczenia)
    vector<double> error;                   // wektor kolumnowy przechowujący dq/d(theta)

    /* wektor kolumnowy przechowujący wagi wejść neuronów wchodzących w skład sieci */
    vector<double> columnWeightVector;  

    /* outputValue - wartość na wyjściu sieci
     * expectedOutputValue - oczekiwana wartość wyjścia
     * inputNumber - numer wejścia neuronu (liczone od 0) 
     * */
    double countOuterError(double outputValue, double expectedOutputValue, int inputNumber)
    {
        int layer=layerInputs.size()-2;

        return (outputValue-expectedOutputValue)*neurons[layer-1][inputNumber].getOutput();
    }

    /* errors - wektor błędów z k+1-szej warstwy
     * layer - numer warstwy w której jesteśmy (k)
     * neuronNumber - numer neuronu z pierwszej warstwy 
     * input - wartość na danym wejściu neuronu, dla którego liczymy błędy
     * */
    double countInnerError(vector<double> errors, int layer, int neuronNumber, double input)
    {
        double s = neurons[layer-1][neuronNumber].getSum();   // suma obliczana przez i-ty neuron k-tej warstwy
        double derivative = exp(s)/ pow(1 + exp(s), 2);

        double sum = 0;                                       // suma error * waga
        vector<double> weightVector;
        for(int i=0;i<layerInputs[layer+1];i++)
        {
            weightVector.push_back(neurons[layer][i].getWeightVector()[neuronNumber]);
        }
        
        double weight;
        for(int i = 0; i < neurons[layer].size(); i++)
        {
            weight = weightVector[i];
            sum += weight * errors[i]/neurons[layer][i].getOutput();
        }
 
        return derivative * input * sum;
    }

    /* errors - wektor błędów z k+1-szej warstwy
     * neuronNumber - numer neuronu z pierwszej warstwy
     * input - wartość na danym wejściu neuronu, dla którego liczymy błędy
     * */
    double countInputError(vector<double> errors, int neuronNumber, double input)
    {
        double s = neurons[0][neuronNumber].getSum();       // suma obliczana przez i-ty neuron k-tej warstwy
        double derivative = exp(s)/ pow(1 + exp(s), 2);

        double sum = 0;                                     // suma error * waga
        vector<double> weightVector;
        for(int i=0;i<layerInputs[2];i++)
        {
            weightVector.push_back(neurons[1][i].getWeightVector()[neuronNumber]);
        }
        
        double weight;
        for(int i = 0; i < neurons[1].size(); i++)
        {
            weight = weightVector[i];
            sum += weight * errors[i]/neurons[1][i].getOutput();
        }

        return derivative * input * sum;
    }

    /*  Metoda wykonująca propagację wsteczną. Wypełnia przy tym wektory kolumnowe wag i dq/d(theta).  */
    void propagateBackwards()
    {
        int startingIndex=0;
        int layer=layerInputs.size()-2;

        for(int i=0;i<layerInputs[layer+1];i++)                        // propagacja po neuronach warstwy wyjściowej
        {
            vector<double> weight = neurons[layer][i].getWeightVector();        // tworzenie wektora kolumnowego wag                                                                     
            columnWeightVector.insert(end(columnWeightVector), begin(weight), --end(weight)); 

            for(int j=0;j<layerInputs[layer];j++)

                error.push_back(countOuterError(networkOutput[i],expectedNetworkOutput[i],j));
        }

        for(int z=layer;z>0;z--)                                          // propagacja po neuronach warstw ukrytych
        {
            for(int i=0;i<layerInputs[z];i++)                                                                                                 
            {
                vector<double> weight = neurons[z-1][i].getWeightVector();      // tworzenie wektora kolumnowego wag                                                                     
                columnWeightVector.insert(end(columnWeightVector), begin(weight), --end(weight)); 

                vector<double> errors;

                for(int x=0;x<layerInputs[z+1];x++)                                  // przygotowanie wektora errors
                {
                    errors.push_back(error[startingIndex+i+(x*layerInputs[z])]); 
                }

                for(int j=0;j<layerInputs[z-1];j++) 
                {
                    if(z>1)
                        error.push_back(countInnerError(errors,z,i,neurons[z-2][j].getOutput()));
                    else
                        error.push_back(countInputError(errors,i,neurons[z-1][0].getInput(j))); 
		        /* liczenie błędów dla wejść sieci (wejście nie zależy od numeru 
			 * neuronu -> dla każdego takie samo)
			 * */
                }
            }
            startingIndex+=layerInputs[z+1]*layerInputs[z];
        }                                                                                       
    }

    /*  Metoda przeprowadzająca sochastyczny najszybszy spadek  */
    void sochasticDescent(double beta)
    {
        for(int i = 0; i < error.size(); ++i)
        {
            columnWeightVector[i] = columnWeightVector[i] - (beta * error[i] * 0.5);
        }

        return;
    }

    /*  Metoda sprawdzająca dokładność przybliżenia wyjścia sieci neuronowej do zadanej oczekiwanej wartości.  */
    bool precisionReached(double epsilon)
    {
        for(int i=0;i<networkOutput.size();i++)
        {
            if(abs(networkOutput[i]-expectedNetworkOutput[i]) / expectedNetworkOutput[i] > epsilon)
                return false;
        }
        return true;
    }

    public:

    vector<double> getErrors(){
        return error;
    }
    vector<double> getWeightsColumnVector(){
        return columnWeightVector;
    }
    int getNumberOfNetworkInputs(){
        return layerInputs[0];
    }
    vector<int> getNumberOfNeurons(){
        vector<int> temp;

        auto it = layerInputs.begin();
        ++it;

        for(it; it < layerInputs.end(); ++it){
            temp.push_back(*it);
        }
        return temp;
    }
    

    /* numberOfParameters - informacja o liczbie parametrów wejściowych 
     * layer - wektor przechwoujący informację o liczbie neuronów w każdej z powłok
     * */
    mlp(int numberOfParameters, vector<int> layer)
    {
        layerInputs.push_back(numberOfParameters);
        layerInputs.insert(end(layerInputs), begin(layer), end(layer));
        int j=0;

        neurons.resize(layer.size());

	// stworzenie neuronów warstw ukrytych
        for(vector<int>::iterator it = layer.begin(); it != layer.end()-1; ++it) 
        {
	    /* resize tutaj nie zadziała, ponieważ zapełni wektor identycznymi neuronami 
	     * (konstruktor zostanie wywołany raz dla każdej warstwy)
	     * */ 
            for(int i=0;i<(*it);i++)   
                neurons[j].push_back(neuron(1,layerInputs[j]));
            j++;
        }

        for(int i=0;i<layer.back();i++)
            neurons[j].push_back(neuron(0,layerInputs[j]));  // stworzenie neuronów (liniowych) warstwy wyjściowej
        
        expectedNetworkOutput.resize(layer.back());          // domyślnie na wyjściu sieci jest wektor zer
        networkOutput.resize(layer.back());
    }

    ~mlp(){};

    /*  Metoda pozwalająca na ustawienie wartości w wektorze wejść.  */
    void setInput(int inputNumber, double value)
    {
        for(int i=0;i<layerInputs[1];i++)
            neurons[0][i].setInput(inputNumber, value);
    }

    void setExpectedOutput(int outputNumber, double value)
    {
        expectedNetworkOutput[outputNumber]=value;
    }

    /*  Metoda symulująca przepływ danych przez perceptron.  */
    void processData()
    {
        double output;
        int end=layerInputs.size();
        vector<double> outputVector;
        for(int i=1;i<end;i++)              // "przejście" przez sieć
        {
            for(int j=0;j<layerInputs[i];j++)
            {
                output=neurons[i-1][j].getNewOutput();
                if(i!=(end-1))
                { 
                    // przekazanie wyjścia danego neuronu do neuronów następnej warstwy
                    for(int z=0;z<layerInputs[i+1];z++)      
                        neurons[i][z].setInput(j, output);
                }
                else
                    outputVector.push_back(output);
            }
        }
        networkOutput.assign(outputVector.begin(), outputVector.end());
    }

    /*  Metoda symulująca przepływ danych przez perceptron i naukę (przykład po przykładzie).  */
    void processDataAndLearn()
    {
        double i=10;

        while(!precisionReached(0.0001) && i<50)
        {
            processData();
            propagateBackwards();
            sochasticDescent(1/i);

            int weightIndex=0;
            for(int z=layerInputs.size()-1;z>0;z--)                         // ustawienie wag
            {
                for(int i=0;i<layerInputs[z];i++)
                {
                    for(int j=0;j<layerInputs[z-1];j++)
                    {
                        neurons[z-1][i].setWeight(j,columnWeightVector[weightIndex]);
                        weightIndex++;
                    }
                }
            }

            error.clear();                                                 // wyczyszczenie buforów
            columnWeightVector.clear();
            i+=0.5;
        }
    }

    /*  Zwraca wektor wyjść danej sieci neuronowej.  */
    vector<double> getOutputVector()
    {
        return networkOutput;
    }

    vector<vector<neuron>> getNeurons(){
        return neurons;
    }

    friend ostream &operator<<( ostream &output, const mlp &network )
    {
        for(double i: network.networkOutput)
            output << i << " ";
    }         
};

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <math.h>

using namespace std;

class neuron{
    vector<double> input;                                           // wektor wejść
    vector<double> weight;                                          // wektor wag
    double output;
    int type, numberOfInputs;                                       // type==0 to neuron liniowy; type==1 to neuron sigmoidalny

    /*
    Sumator
    */
    double sum()
    {
        double x=0;
        for(int i=0;i<numberOfInputs;i++)
             x+=input[i]*weight[i];

        return x;
    }

    /*
    Funkcja aktywacyjna
    */
    double applyFunction()
    {
        if(type==0)
            return sum();
        else
        {
            double z=sum();
            return exp(z)/(1+exp(z));
        }
    }

    public:
    neuron()
    {
        type=0;
        numberOfInputs=1;
        input.resize(1, 1.0);

        default_random_engine rand_num{static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
        uniform_real_distribution<float> dis(-0.18, 0.18);                          // ∼U (−1/√dim(we),1/√dim(we)) - około (-0,18;0,18)
        weight.resize(1, dis(rand_num));                                              
    }

    neuron(int type, int numberOfInputs)
    {
        this->type=type;
        this->numberOfInputs=numberOfInputs+1;
        input.resize(this->numberOfInputs, 0.0);
        input[numberOfInputs]=1;
        weight.resize(this->numberOfInputs);

        default_random_engine rand_num{static_cast<long unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count())};
        this_thread::sleep_for (chrono::milliseconds(1));                     // potrzebne krótkie oczekiwanie, aby seed był inny przy tworzeniu wielu neuronów w krótkim odstępie czasowym
        uniform_real_distribution<float> dis(-0.18, 0.18);                    // ∼U (−1/√dim(we),1/√dim(we)) - około (-0,18;0,18)

        for(int i=0;i<this->numberOfInputs;i++)
            weight[i]=dis(rand_num);
    }

    ~neuron(){};

    /*
    Zwraca liczbę "edytowalnych" wejść -> ostatnie wejście ma wartość 1.
    */
    int getNumberOfInputs()
    {
        return numberOfInputs-1;
    }

    void setInput(int inputNumber, double value)
    {
        input[inputNumber]=value;
    }

    void setWeight(int inputNumber, double value)
    {
        weight[inputNumber]=value;
    }

    vector<double> getWeightVector()
    {
        return weight;
    } 

    /*
    Wyjście neuronu po przetworzeniu wejść w sumatorze i po zastosowaniu funkcji aktywacyjnej.
    */
    double getOutput()
    {
        return applyFunction();
    }
};
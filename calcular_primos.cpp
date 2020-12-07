/*
	Projeto para calcular quantidade de numeros primos baseado em quantidade de interacoes
*/
#include <mpi.h>


/*
	Libs de IO
*/
#include <stdio.h>
#include <iostream>

/*
	Libs de operacoes matematicas
*/
#include <cstdlib>
#include <cmath>


#include <vector>

using namespace std;
bool valida_primo(int num)
{	
	if (num%2==0)
	{
		return 0;
	}
	for (int i=3;i<sqrt(num);i+=2)
	{
		if (num%i==0)
		{
			return 0;
		}
	}
	return 1;
}
/*
int get_factorial(int num)
{
	int retorno = 0;
	for(int i=num; i>0; i--)
		retorno*=i

	return retorno
}
*/

int numero_de_processos;

int *numeros_primos;

int *aux_numeros_primos;
int aux_acum_pos;

vector <int> acum_primos;

int n;

int main(int argc, char** argv) {

	int i;
	/*
		Validando se temos os 2 parametros de entrada
		1 = Padrao 
		n = Numero de iteracoes

	*/
	if (argc!=2)
	{
		cout <<" Numero de argumentos apenas o numero de iteracoes"<<endl;
		return 0;
	}
	// Calculando um n para servir de iteracoes
	n = 100000 * atoi(argv[1]);
	
	// O numero de primos disponiveis ja sera esperado que nao tenham os pares ou seja 1/2 do valor total
	numeros_primos = new int[n/2];

	MPI_Init(NULL, NULL);

	// Solicitando a quantidade de nodes + master necessário
	MPI_Comm_size(MPI_COMM_WORLD, &numero_de_processos);

	// Localizando em que "Trecho"/"No"/"Node" estamos executando
	int id_processo;
	MPI_Comm_rank(MPI_COMM_WORLD, &id_processo);


	// Dividindo a responsabilidade/Range de quantos numero serao testado se sao primos em cada particao
	for (i = n*id_processo / numero_de_processos;i < n*(id_processo + 1) / numero_de_processos; i++)
		if (valida_primo(i))
			numeros_primos[aux_acum_pos++]=i;
	// if we are not in the main process(process with id_processo 0) send our prime numbers to main process
	/* 
		Se o processo for id "0" significa ser a nossa master
		Caso nao seja a master devera responder os numeros primos ao master
	*/
	if (id_processo!=0)
	{
		MPI_Send(&aux_acum_pos, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(numeros_primos, aux_acum_pos, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}


	int num_primos_nodes;



	if (id_processo==0)
	{
		// Se estivermos no Master
		// Acumulamos todos os primos calculados nos nodes (paralelo ao reduce)
		// Analogo ao Array de Arrays
		for (i=0;i<aux_acum_pos;i++)    	
			acum_primos.push_back(numeros_primos[i]);
		
		delete numeros_primos;
		// Agora para cada numero em cada Array de recebido pelos nodes
		// Serra "Appendado" no nosso vetor na Master
		for (i=1;i<numero_de_processos;i++)
		{
			MPI_Recv(&num_primos_nodes, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			cout <<"Node ("<<i<<"/"<<numero_de_processos-1<<") Devolveu "<<num_primos_nodes<<" primos "<<endl;
			aux_numeros_primos = new int[num_primos_nodes];
			MPI_Recv(aux_numeros_primos, num_primos_nodes, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (int j=0;j<num_primos_nodes;j++)
				acum_primos.push_back(aux_numeros_primos[j]);
			delete aux_numeros_primos;
		}
		
		cout <<" Ao todo "<<acum_primos.size()<<" primos"<<endl;
		
		cout <<"Escrevendo todos os primos no todos_primos.txt"<<endl;
		
		freopen("todos_primos.txt","w",stdout);
		for(i=0;i<acum_primos.size();i++)
			printf("%d\n", acum_primos[i]);
		
		/*
		cout <<" Printando todos os primos calculados: "<<endl;
		for(i=0;i<acum_primos.size();i++)
			cout <<acum_primos[i]<<endl;

		*/
	}



	MPI_Finalize();
	return 0;
}
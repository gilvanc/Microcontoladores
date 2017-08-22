//Exemplo: Ajuste de Modelo por Mínimos Quadrados
//%Programa:MQ.sce
clear;

//Pontos Experimentais
yp = [ 0.1 2.1 2.7 3.1 3.8 4.2]; //Corrente [A]
xp = [ 0 1 2 3 4 5 ]; //Tensão [V]
plot(xp,yp,'or');

//Modelo Adotado: y = k*(x^p)
p = 3/5; //Expoente Fracionário (Modelo)
g = xp.^p; //Função Base
k = sum(yp.*g)/sum(g.*g); //Ajuste da Constante do Modelo

xc = linspace(min(xp),max(xp),100); //Base de Plotagem do Modelo Ajustado
yc = k*(xc.^p); //Modelo Ajustado
plot(xc,yc,'b');
title('Modelo Ajustado (azul) e Pontos Experimentais (vermelho)');


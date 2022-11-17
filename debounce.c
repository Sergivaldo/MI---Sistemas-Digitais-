/*
  ----------- Resposta do #Desafio 2 --------------
  =================================================
  ==   BLOG DA ROBOTICA - www.blogdarobotica.com ==
  =================================================
  Autor: Kleber Bastos
  E-mail: contato@blogdarobotica.com
  Fanpage: facebook.com/blogdarobotica
  YouTube: youtube.com/user/blogdarobotica
  Instagram: instagram.com/blogdarobotica
*/

int statusBotao;                       
int ultimoStatusBotao = 1;           

unsigned long tempoUltimoDebounce = 0; //Variável que armazenará o tempo do último Debounce
unsigned long tempoDebounce = 50;      //Tempo (em milissegundos)que é necessário manter o botão pressionado para ter a certeza de que ele foi pressionado de fato.

//Função responsável por fazer o Debounce
void debounceBotao() {

  int leitura = digitalRead(botao);                       //A variável leitura recebe a leitura do pino do botão: HIGH (pressionado) ou LOW (Desacionado)

  if (leitura != ultimoStatusBotao) {                     //Se a leitura atual for diferente da leitura anterior
    tempoUltimoDebounce = millis();                       //Reseta a variável tempoUltimoDebounce atribuindo o tempo atual para uma nova contagem
  }

  if ((millis() - tempoUltimoDebounce) > tempoDebounce) { //Se o resultado de (tempo atual - tempoUltimoDebounce) for maior que o tempo que determinamos (tempoDebounce), ou seja, já passou os 50 milissegundos que é o tempo que o botão precisa ficar pressionado para ter a certeza de que ele realmente foi pressionado? Se sim faça:
    if (leitura != statusBotao) {                         //Verifica se a leitura do botão mudou, ou seja, se é diferente do status que o botão tinha da última vez. Se sim, faça:
      statusBotao = leitura;                              //statusBotao recebe o que foi lido na variável leitura (pressionado = 1 e não pressionado = 0)
      if (statusBotao == HIGH) {                          //Se o statusBotao é igual a HIGH significa que o botão foi pressionado, então faça:
      }
    }
  }
  ultimoStatusBotao = leitura;                            //Atualiza a variável ultimoStatusBotao para o que foi lido na variável leitura

}

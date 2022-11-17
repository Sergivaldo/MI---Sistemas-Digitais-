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

int buttonStatus;                       
int lastButtonStatus = 1;           

unsigned long lastDebounceTime = 0; //Variável que armazenará o tempo do último Debounce
unsigned long debounceTime = 50;      //Tempo (em milissegundos)que é necessário manter o botão pressionado para ter a certeza de que ele foi pressionado de fato.

//Função responsável por fazer o Debounce
void debounce() {

  int reading = digitalRead(btt);                       //A variável leitura recebe a leitura do pino do botão: HIGH (pressionado) ou LOW (Desacionado)

  if (reading != lastButtonStatus) {                     //Se a leitura atual for diferente da leitura anterior
    lastDebounceTime = millis();                       //Reseta a variável tempoUltimoDebounce atribuindo o tempo atual para uma nova contagem
  }

  if ((millis() - lastDebounceTime) > debounceTime) { //Se o resultado de (tempo atual - tempoUltimoDebounce) for maior que o tempo que determinamos (tempoDebounce), ou seja, já passou os 50 milissegundos que é o tempo que o botão precisa ficar pressionado para ter a certeza de que ele realmente foi pressionado? Se sim faça:
    if (reading != buttonStatus) {                         //Verifica se a leitura do botão mudou, ou seja, se é diferente do status que o botão tinha da última vez. Se sim, faça:
      buttonStatus = reading;                              //statusBotao recebe o que foi lido na variável leitura (pressionado = 1 e não pressionado = 0)
      if (buttonStatus == 1) {                          //Se o statusBotao é igual a HIGH significa que o botão foi pressionado, então faça:
      }
    }
  }
  lastButtonStatus = reading;                            //Atualiza a variável ultimoStatusBotao para o que foi lido na variável leitura

}

<a id="inicio"></a>
## Mi Sistemas digitais - Problema 3

A Internet das Coisas (IoT) descreve a rede de objetos físicos incorporados a sensores, software e outras tecnologias com o objetivo de conectar e trocar dados com outros dispositivos e sistemas pela internet. Com isso, pode-se desenvolver diversos projetos de automação, como por exemplo, casas inteligentes que podem ser controladas e monitoradas por aplicações, sejam essas mobile, web ou desktop. O 3° problema da disciplina de Sistemas Digitais tem como proposta a implementação de um sistema de sensoriamento utilizando o protocolo MQTT para fazer o tráfego de dados dos sensores pela internet. A aplicação possui duas IHMs(Interface homem-máquina) para que o usuário possa interagir com o programa tanto localmente(através de botões e um display lcd) e remotamente(por uma interface web). Sendo assim, o usuário irá ter acesso a informações como: valor de sensores, acionamento de leds. histório das medições recebidas e também o estado da conexão com a estação de medição.


### Membros do grupo
  **Jacob Santana**<br>
  **Sergivaldo Junior**

## Seções

Abaixo as seções que podem ser navegadas para um maior entendimento da solução encontrada, desde as ferramentas utilizadas até os testes feitos.

&nbsp;&nbsp;&nbsp;[**1.** Protocolo MQTT](#secao5)

&nbsp;&nbsp;&nbsp;[**2.** Diagrama do projeto](#secao4)

&nbsp;&nbsp;&nbsp;[**4.** Metodologias e técnicas aplicadas](#secao7)

&nbsp;&nbsp;&nbsp;[**5.** Testes realizados](#secao8)

&nbsp;&nbsp;&nbsp;[**6.** Limitações do projeto](#secao9)

## Protocolo MQTT

Para fazer a comunicação da estação de medição com as IHMs, foi utilizado o protocolo MQTT para a troca de dados entre essas entidades. O MQTT funciona
no formato Cliente/Servidor, onde pode-se ter vários clientes conectados ao servidor que permite que esses troquem mensagens entre si. Para que um cliente possa receber apenas mensagens que lhe interesse é implementado o modelo publisher/subscriber onde um cliente pode assumir um desses papéis ou os dois. Como publisher, o cliente poderá enviar mensagens para um determinado endereço chamado tópico. Como subscriber, o cliente poderá se inscrever em um tópico para receber as mensagens que chegarem. Como não há uma conexão direta entre o publisher e o subscriber, é necessário que se tenha um intermediário para fazer o gerenciamento das mensagens, este é chamado de broker.
Na solução feita há três clientes conectados, sendo estes a estação de medição(NodeMCU), a Raspberry Pi e a aplicação web. Ambas utilizam do mesmo broker para se comunicarem, entretanto, como as informações são centralizadas na Raspberry Pi, a interface web nunca deverá se comunicar diretamente com a estação de medição, apenas a SBC possui essa permissão. Sendo assim, existem tópicos específicos para a comunicação entre a NodeMCU e a Raspberry e entre a Raspberry e o aplicativo web.

### Tópicos utilizados para a comunicação

| **Endereços**                      	|                                                              **Finalidade** 	|
|------------------------------------	|----------------------------------------------------------------------------:	|
| tp04/g03/mqtt/response/value       	|                Tópico no qual a NodeMCU responde a requisições da Raspberry 	|
| tp04/g03/mqtt/request/value        	|                          Tópico no qual a Raspberry solicita algo a NodeMCU 	|
| tp04/g03/node/analog-sensor/value  	|       Tópico onde será enviado o valor do sensor analogico para a Raspberry 	|
| tp04/g03/node/digital-sensor/value 	|              Tópico onde será enviado o valor de todos os sensores digitais 	|
| tp04/g03/node/status               	|        Tópico onde a node enviará uma resposta indicando que está conectada 	|
| tp04/g03/node/active-sensors       	|              Tópico onde será enviado os sensores digitais que estão ativos 	|
| tp04/g03/node/time-interval        	|   Tópico onde será enviado o intervalo de tempo das medições para a NodeMCU 	|
| tp04/g03/mqtt/response/rasp/value  	|        Tópico onde a Raspberry irá responder a requisições da aplicação web 	|
| tp04/g03/app/status                	|   Tópico onde a aplicação enviará uma resposta indicando que está conectada 	|
| tp04/g03/mqtt/request/app/value    	|                         Tópico onde a aplicação solicitará algo a Raspberry 	|
| tp04/g03/mqtt/response/app/value   	|           Tópico onde a aplicação responderá a uma solicitação da Raspberry 	|
| tp04/g03/app/time-interval         	| Tópico onde será enviado o intervalo de tempo das medições para a RaspBerry 	|
| tp04/g03/app/active-sensors        	|                Tópico onde será enviado para a Raspberry os sensores ativos 	|
| tp04/g03/app/analog-sensor/value   	|   Tópico onde será enviado para a aplicação o valor dos sensores analógicos 	|
| tp04/g03/app/digital-sensor/value  	|     Tópico onde será enviado para a aplicação o valor dos sensores digitais 	|
| tp04/g03/app/history               	|           Tópico onde será enviado para a aplicação histórico das medições  	|

## Diagrama do projeto

![Untitled](https://user-images.githubusercontent.com/72475500/208279012-3b024b4e-e8bc-4473-86b7-f7ed77cc7585.png)

Acima é mostrado a organização do projeto em relação ao protocolo MQTT. A comunicação geral do sistema é feita com o uso desse protocolo. A estação de medição NodeMCU conversa através do broker com a IHM local e detentora das informações, a Raspberry Pi, e esta conversa pelo mesmo broker, mas por tópicos diferentes com a aplicação web. Dessa forma mantendo a idéia de centralização das informações dos sensores na SBC.
No broker, será utilizado de endereços para o envio de requisições, respostas as requisições, valores dos sensores analogicos e digitais e testes de conexão, existindo tópicos exclusivos para cada um destes. Foi utilizada essa estratégia para facilitar o envio das mensagens, já que cada uma terá formatos específicos o que faria que o tratamento das mensagens se tornasse algo trabalhoso.


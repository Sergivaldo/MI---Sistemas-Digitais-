<a id="inicio"></a>
## Mi Sistemas digitais - Problema 3

A Internet das Coisas (IoT) descreve a rede de objetos físicos incorporados a sensores, software e outras tecnologias com o objetivo de conectar e trocar dados com outros dispositivos e sistemas pela internet. Com isso, pode-se desenvolver diversos projetos de automação, como por exemplo, casas inteligentes que podem ser controladas e monitoradas por aplicações, sejam essas mobile, web ou desktop. O 3° problema da disciplina de Sistemas Digitais tem como proposta a implementação de um sistema de sensoriamento utilizando o protocolo MQTT para fazer o tráfego de dados dos sensores pela internet. A aplicação possui duas IHMs(Interface homem-máquina) para que o usuário possa interagir com o programa tanto localmente(através de botões e um display lcd) e remotamente(por uma interface web). Sendo assim, o usuário irá ter acesso a informações como: valor de sensores, acionamento de leds. histório das medições recebidas e também o estado da conexão com a estação de medição.


### Membros do grupo
  **Jacob Santana**<br>
  **Sergivaldo Junior**

## Seções

Abaixo as seções que podem ser navegadas para um maior entendimento da solução encontrada, desde as ferramentas utilizadas até os testes feitos.

&nbsp;&nbsp;&nbsp;[**1.** Protocolo MQTT](#protocolo-mqtt)

&nbsp;&nbsp;&nbsp;[**2.** Diagrama do projeto](#diagrama-do-projeto)

&nbsp;&nbsp;&nbsp;[**3.** Metodologias e técnicas aplicadas](#metodologias-e-técnicas-aplicadas)

&nbsp;&nbsp;&nbsp;[**4.** Testes realizados](#testes-realizados)

&nbsp;&nbsp;&nbsp;[**5.** Limitações do projeto](#limitações-do-projeto)

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

## Metodologias e Técnicas aplicadas

### Manipulação de strings

Para fazer o envio das informações, foi necessário utilizar de strings. Sendo assim, algumas manipulações um pouco mais complexas foram utilizadas para
pegar algumas informações corretamente. A situação mais importante em que essa estratégia foi utilizada, foi no envio do valor dos sensores digitais, já que
a medição de todos os sensores são emitidas em uma única mensagem. Esses valores foram colocados no seguinte formato `D0-n,D1-n,D2-n,D3-n,D4-n,D5-n,D6-n,D7-n` 
onde *Dx* especifica a numeração do sensor digital e o *n* o seu valor. Esse formato é bastante útil pois quando recebido na SBC, a string será fatiada e o valor de cada sensor irá para a posição em um array correspondente com seu número. Exemplo: D0 = Array\[0].

### Algoritmo de fila 

![Frame 1](https://user-images.githubusercontent.com/72475500/208280521-79805462-588f-4b4b-9a3a-9ac873138da4.png)

O sistema permite o salvamento das últimas 10 medições, para fazer a atualização desses dados no momento que  tiver 10 históricos, foi utilizado o algoritmo de fila
onde o primeiro a entrar é o primeiro a sair. Sendo assim, o histórico mais antigo é removido e os outros históricos são movidos, fazendo com que a 10 posição agora fique disponível para o próximo histórico.

## Testes Realizados

### Envio e recebimento de medições

Durante o teste foi possível observar que os dados dos sensores foram recebidos corretamente na SBC. Quando pressionado um push-button(sensor digital) os dados eram recebidos fielmente, emitindo 0 quando clicado e 1 quando solto. No sensor analógico(potenciômetro) foi observado o mesmo, apesar de que ocorriam algumas variações na medição mesmo quando o periférico não era mexido e também as vezes o sensor não conseguia alcançar seu valor máximo(1024) ou mínimo(0), variando apenas entre 10 e 1020, ainda assim, esses valores correspondiam com o que a estação de medição recebia.

### Intervalo de medições

Tanto na Raspberry como na NodeMCU foi colocado um intervalo padrão, intervalo esse que pode ser tranquilamente modificado para outro valor. Essa modificação é feita com excelência, assim que a estação recebe o valor, ele será modificado para o novo. Durante os testes, pôde-se perceber que quando resetada a estação de medição, o valor definido para o intervalo era ignorado, o que fazia com que a medição fosse feita muito rápida, comprometendo a visualização dos valores no display lcd. Outro adendo é que o momento em que o valor do sensor é modificado, pode causar a impressão de que o intervalo esteja mais rápido, o que não é verdade, apenas o valor que foi mudado em um momento muito próximo do envio das medições.

### Histórico das medições

O histórico das medições mostra os dez últimos valores dos sensores analógicos e digitais, além do horário em que foram recebidos. Uma observação a ser feita é que
o horário pode ser registrado errado caso este, que é pego da própria SBC não esteja configurado, o que faz perder a consistência em uma parte da informação. Isso poderia ser melhorado caso o horário fosse pego através de um servidor NTP.

### Mudança de estado do led da NodeMCU

O estado do led é mudado através de uma requisição que a Raspberry faz para a NodeMCU, no momento que a requisição chega, a NodeMCU verifica se o comando corresponde com o acendimento do led e executa a tarefa, posteriormente retornando uma resposta com o estado atual do led(Ligado ou desligado). Analisando essa funcionalidade, foi visto que esta executava corretamente, mas ainda possui problema com a unicidade, pois, como a SBC pode vir a se desconectar do Broker ou desligar, ela irá perder o estado em que o led se encontrava já que seus dados não persistem. Assim, a Raspberry Pi irá conter um dado que não corresponde com o real, já que o estado do led da estação de medição  na Raspberry inicia como apagado, mas na NodeMCU pode estar ligado, fazendo com que a veracidade das informações seja perdida. Para resolver esse problema foi implementado que ao se conectar com o broker, a SBC irá solicitar o estado do led fazendo com que o estado real seja exibido para o usuário.

### Especificação dos sensores ativos

Por padrão, foram colocados como ativas as portas digitais D0 e D1, já que foram estas as que possuiam sensores ligados a elas. Basicamente, quando ativada uma porta digital, a estação de medição irá verificar os valores do sensor que estiver nessa porta. Foi analisado que quando uma porta que não está sendo utilizada é ativada, por padrão será enviado o valor zero.

### Teste de conexões

Na implementação dessa funcionalidade foi utilizado uma thread para que a conexão fosse testada. Neste procedimento, a thread envia uma mensagem para a estação de medição e aguarda uma resposta desta. Quando uma mensagem chega, uma váriavel irá mudar o seu valor para o que indica a chegada da resposta, se o tempo limite acabar e esse valor não estiver na variável, quer dizer que nenhuma mensagem foi recebida e a nodeMCU não está conectada. Nas observações feitas, esses processo se saiu bem, indicando o status de conexão tanto da estação, quanto do aplicativo.

## Limitações do projeto

### Horário do histórico é o de recebimento e não da medição
Devido a estação de medição não possuir um módulo RTC(Real Time Clock) ou acesso a um servidor de horário devido a limitações da rede, o horário presente em cada
histórico é o de recebimento das mensagens, isso pode fazer com que o horário não seja perfeitamente preciso. Além disso, o horário da SBC no qual é utilizado não consegue se manter configurado, por que sofre das mesmas necessidades da NodeMCU, então, mesmo o configurando, no momento que a SBC é desligada, o horário correto é perdido.

### IHM remota incompleta

A IHM remota, no qual além de possuir todas as funcionalidades da IHM local, mostrava também um histórico no formato de gráfico temporal não foi completada, podendo utilizá-la apenas para ver o status das conexões e o estado do led.

### Intervalo pequeno de tempo

Apesar de poder selecionar a unidade de tempo(segundo, minuto ou hora) o valor era limitado a no máximo 10 o que para determinados sensores poderia não ser muito eficaz já que o valor necessário não estaria disponível.

[Topo](#inicio)

<a id="inicio"></a>
## Mi Sistemas digitais - Problema 3

Manual do Usuário - IHM local

Este é protótipo de sistema de sensoriamento genérico montado em uma plataforma baseada na NodeMCU. O sistema é comandado pelo Single Board Computer (SBC) Orange Pi e conta com um aplicativo e com uma NodeMCU, a comunicação entre eles é realizada utilizando o protocolo MQTT. 

Este manual tem o intuito de orientar a utilização da IHM(Interface Homem-Máquina) local, que foi implementado na Orange Pi.

### Membros do grupo
  **Jacob Santana**<br>
  **Sergivaldo Junior**
  <a id="inicio"></a>

## Seções

&nbsp;&nbsp;&nbsp;[**1.** Instalação do sistema](#secao1)

&nbsp;&nbsp;&nbsp;[**2.** Periféricos de interação](#secao2)

&nbsp;&nbsp;&nbsp;[**3.** Opções do menu](#secao3)

<a id="secao1"></a>
## Instalação do sistema

O sistema utiliza 3 plataformas para o seu funcionamento, sendo centralizado na Orange Pi, utilizando a NodeMCU para realizar as leituras e o aplicativo para monitoramento e configurações do sistema. As plataformas se comunicam utilizando um broker, através do protocolo MQTT.

Para realizar a instalação na Orange Pi, é necessário conectá-la na mesma rede que o broker, além de instalar a biblioteca Paho-mqtt nesta placa, feito isso, basta executar o arquivo main.c presente neste repositório.

Para realizar a instalação na NodeMCU, deve ser utilizada a IDE do arduino. Inicialmente, as bibliotecas PubSubClient e TimedAction na IDE do arduino. Feito isso, basta executar o arquivo node_main.c presente neste repositório.

A instalação do aplicativo pode ser realizada acessando o repositório:https://github.com/Sergivaldo/IOT-Remote


<a id="secao2"></a>
## Periféricos de interação
Para a interação com a interface local, são utilizados 3 botões e o display lcd. 

![Funky Bombul-Bojo](https://user-images.githubusercontent.com/72475500/208282067-49db5eaf-8ca6-47db-b0fd-921e982133dd.png)

Os botões 1 e 2 são utilizados para a percorrer para a esquerda e para a direita respectivamente, enquanto o terceiro botão têm funcionalidades diferentes dependendo do menu atual.

<a id="secao3"></a>
## Opções do menu

#### Tela inicial
 Quando o sistema é iniciado, uma tela inicial é exibida no display lcd. A tela inicial exibe “MI - SD Problema 3” no display lcd, a partir dela podem ser utilizados os botões 1 e 2 para percorrer para as próximas opções do menu, o botão 3 não tem funcionalidade nessa tela. Uma vez que sair dessa tela, não é possível retornar para ela, portanto ela só é exibida uma vez a cada execução do sistema. 


#### Leitura: Sensor digital
A opção de leitura dos sensores digitais permite a visualização em tempo real das medições que são realizadas periodicamente no intervalo pré definido.

Para acessar a medição dos sensores, o botão 3 deve ser clicado quando estiver nessa tela. A medição de cada sensor é exibida de forma individual,  onde na linha de cima do display lcd é exibida a identificação do sensor, e na linha de baixo a sua respectiva medição, caso esse esse sensor esteja ativo, caso contrário, é exibida a mensagem de que ele se encontra desativado. 

Para percorrer entre os sensores, são utilizados os botões 1 e 2, após percorrer todos os sensores no sentido horário (com o botão 2), ou percorrendo uma vez, a partir do primeiro no sentido anti horário(com o botão 1), é exibida a opção de sair desse menu, que pode ser selecionada utilizando o botão 3. 


#### Leitura: Sensor Analogico
A opção de leitura do sensor analógico permite a visualização em tempo real da medição que é realizada periodicamente no intervalo pré definido. A medição pode ser acessada pressionando o botão 3 nesta opção.

Na linha de cima do display lcd é exibida a identificação do sensor analógico, nesta configuração do sistema, só existe uma entrada para leitura analógica, logo, só há um sensor sendo monitorado. Na linha de baixo do display lcd é exibido o valor lido. 

Utilizando os botões 1 ou 2, é exibida a opção de sair desse menu, que pode ser selecionada utilizando o botão 3. 

#### LED
Esta opção do menu permite ligar e desligar o LED da NodeMCU. A visualização do estado atual do LED pode ser vista na emulação da chave exibida no display. quando a chave está para a esquerda, na posição ON, o LED está ligado. Quando a chave está para a direita, na posição OFF, o LED está desligado.

Para acender ou desligar o LED, deve ser pressionado o botão 3, este serve para alternar o estado atual do LED. 


#### Configurações
Na opção de configurações, podem ser definidos o intervalo de tempo em que os sensores serão atualizados, e os sensores que estarão ativados e desativados. Para acessar essas opções, deve ser pressionado o botão 3 enquanto estiver nesta tela. E para percorrer os submenus devem ser utilizados os botões 1 e 2, para selecionar uma opção, deve ser pressionado o botão 3. 

**Ajustar intervalo:** A primeira opção que é exibida é a de “Ajustar  intervalo”. Esta opção é responsável por permitir que o usuário defina o intervalo de tempo em que os sensores realizarão uma nova medida. Para acessar o ajuste, deve ser utilizado o botão 3 nesta opção. 
O ajuste do intervalo pode ser realizado utilizando os botões 1 e 2 para diminuir e aumentar respectivamente as barrinhas do medidor de tempo, cada barrinha é equivalente a uma unidade de tempo pré definido, podendo variar de 1 a 10 unidades, a unidade de tempo é exibida ao lado(s = segundos, m = minutos, h = horas). Após definir o intervalo, o botão 3 deve ser pressionado para confirmar e sair da tela de ajuste.

**Ajustar Unidade de Tempo:** Esta opção, exibida como “Ajustar Unid. Tempo”, permite que o usuário defina a unidade de tempo que o intervalo irá seguir. Para acessar esta opção, deve ser pressionado o botão 3. 
Para alternar a unidade de tempo devem ser utilizados os botões 1 e 2, para confirmar a opção desejada deve ser pressionado o botão 3.

**Especificar Sensores Ativos:** Nesta opção é possível fazer a ativação e a desativação dos sensores que serão monitorados. Para acessar este menu, deve ser pressionado o botão 3. 
Após acessar este menu, os sensores digitais serão exibidos individualmente, cada sensor será exibido com sua identificação na linha de cima do display e seu status de ativação na linha de baixo. Para navegar entre os sensores podem ser utilizados os botões 1 e 2, para percorrer em sentido anti horários e horário respectivamente. Para mudar o status de um sensor, deve ser pressionado o botão 3 quando ele estiver sendo exibido no display, se ele estiver ativado, quando pressionando ele será desativado, e vice-versa. Após definir os sensores, deve ser pressionado o botão 3 na opção de “sair”, para salvar a configurações e sair. 


#### Status das conexões
A opção de status da conexão permite a visualização das conexões do aplicativo e da NodeMCU com o SBC central, para acessar esse menu, deve ser pressionado o botão 3. 

Após acessar este menu, a situação das comunicações serão exibidas de forma individual, onde cada tela exibe a identificação na linha de cima do display lcd, e  o status da conexão na linha de baixo, podendo ser online ou offline. Para navegar entre as exibições, devem ser utilizados os botões 1 e 2, e para sair desse menu deve ser pressionado o botão 3 na opção “sair”.

#### Histórico
A opção do Histórico é responsável por exibir os histórico com as últimas 10 medições dos sensores digitais e do sensor analógico. Para acessá-los, o botão 3 deve ser pressionado.

Após acessar o histórico, é possível visualizar o histórico dos sensores digitais, o histórico do sensor analógico e a opção de sair, devem ser utilizados os botões 1 e 2 para alternar entre as opções e o botão 3 para selecionar a opção desejada. Dentro da opção referente ao histórico dos sensores digitais, cada medição é exibida de forma individual, onde na linha de cima são exibidos as medições de todos os sensores digitais, do D0 ao D8 respectivamente, e na linha abaixo é exibido o número do histórico e o horário em que foi realizada a leitura. A medição dos sensores desativados são exibidas como “n”. para navegar entre os 10 históricos devem ser utilizados os botões 1 ou 2, para sair basta pressionar o botão 3 a qualquer momento. O histórico do sensor analógico utiliza os mesmos mecanismos. 

#### Sair 
Ao pressionar o botão 3 na opção “Sair”, o sistema é encerrado. 


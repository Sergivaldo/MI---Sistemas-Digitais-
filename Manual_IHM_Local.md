<a id="inicio"></a>
## Mi Sistemas digitais - Problema 3

Manual do Usuário - IHM local

Este é protótipo de sistema de sensoriamento genérico montado em uma plataforma baseada na NodeMCU. O sistema é comandado pelo Single Board Computer (SBC) Orange Pi, e comunicação sem fio com um aplicativo e a NodeMCU. 

Este manual tem o intuito de orientar a utilização da IHM(Interface Homem-Máquina) local, que foi implementado na Orange Pi.

### Membros do grupo
  **Jacob Santana**<br>
  **Sergivaldo Junior**
  <a id="inicio"></a>

## Seções

&nbsp;&nbsp;&nbsp;[**1.** Periféricos de interação](#secao1)

&nbsp;&nbsp;&nbsp;[**2.** Opções do menu inicial](#secao2)


<a id="secao1"></a>
## Periféricos de interação
Para a interação com a interface local, são utilizados 3 botões e o display lcd. 

Os botões 1 e 2 são utilizados para a percorrer para a esquerda e para a direita respectivamente, enquanto o terceiro botão têm funcionalidades diferentes dependendo do menu atual.

<a id="secao2"></a>
## Opções do menu inicial

#### Tela inicial

 Quando o sistema é iniciado, uma tela inicial é exibida no display lcd. A tela inicial exibe “MI - SD Problema 3” no display lcd, a partir dela podem ser utilizados os botões 1 e 2 para percorrer para as próximas opções do menu, o botão 3 não tem funcionalidade nessa tela. Uma vez que sair dessa tela, não é possível retornar para ela, portanto ela só é exibida uma vez a cada execução do sistema. 

**Leitura: Sensor digital**

A opção de leitura dos sensores digitais permite a visualização em tempo real das medições que são realizadas periodicamente no intervalo pré definido.

Para acessar a medição dos sensores, o botão 3 deve ser clicado quando estiver nessa tela. A medição de cada sensor é exibida de forma individual,  onde na linha de cima do display lcd é exibida a identificação do sensor, e na linha de baixo a sua respectiva medição, caso esse esse sensor esteja ativo, caso contrário, é exibida a mensagem de que ele se encontra desativado. 

Para percorrer entre os sensores, são utilizados os botões 1 e 2, após percorrer todos os sensores no sentido horário (com o botão 2), ou percorrendo uma vez, a partir do primeiro no sentido anti horário(com o botão 1), é exibida a opção de sair desse menu, que pode ser selecionada utilizando o botão 3. 

**Leitura: Sensor Analogico**

A opção de leitura do sensor analógico permite a visualização em tempo real da medição que é realizada periodicamente no intervalo pré definido.

Na linha de cima do display lcd é exibida a identificação do sensor analógico, nesta configuração do sistema, só existe uma entrada para leitura analógica, logo, só há um sensor sendo monitorado. Na linha de baixo do display lcd é exibido o valor lido. 

utilizando os botões 1 ou 2, é exibida a opção de sair desse menu, que pode ser selecionada utilizando o botão 3. 

**LED**

Esta opção do menu permite ligar e desligar o LED da NodeMCU. A visualização do estado atual do LED pode ser vista na emulação da chave exibida no display. quando a chave está para a esquerda, na posição ON, o LED está ligado. Quando a chave está para a direita, na posição OFF, o LED está desligado.

Para acender ou desligar o LED, deve ser pressionado o botão 3, este serve para alternar o estado atual do LED. 

**Configurações**

**Status das conexões**

**Histórico**

**Sair**
# Rootkit
Um RootKit (simples) & Funcional para exemplos de persistência. Ele foi feito em apenas 1 dia, então desculpe-me se algo estiver errado ou esqueci de melhorar alguma coisa.

## Base
Um rootkit é um tipo de malware que se instala e se oculta da máquina, tornando-o muito difícil de remover. Neste exemplo, quis dar uma demonstração de que, se bem feito, um rootkit pode ser uma ameaça extremamente difícil de ser removida.

## Gráfico
![RootKit](https://github.com/elDimasX/Rootkit/assets/51800283/c8ac4b2c-8790-4811-9e8c-9256a663da5b)
<br/>Apenas um simples exemplo, em gráfico, de como este rootkit funciona.

## Explicação de funcionamento
Esse rootkit foi feito apenas visando para se permanencer na máquina (o que deve ser o principal objetivo de um rootkit), então, não há malware ou algo que danifique a máquina (recomendo não instalar o modo persitência para isso). Aqui está a lógica passo-a-passo:
<br/>
1. O programa inicia, e quando clica em 'Instalar Rootkit', ele instala o arquivo .inf, inicia o driver, pede para desativar a autoproteção de registro, e modifica o ImagePath para 'RootKit.sys ', para que ele carregue depois
2. Quando o driver inicia, ele se auto-renomeia para 'RootKit.sys ' com um espaço no final. Isso faz bugar o nome de arquivo, e o Windows, e muitos outros programas não conseguem remove-lo
3. O driver usa um minifiltro, então sempre que algum processo acessa ele, ele o finaliza imediatamente
4. O driver também inicia no boot, o que o torna mais difícil ainda de ser removido

<br/>Lembre-se de que o foco deste rootkit é apenas tentar permanecer na máquina, e nada mais. Existe muitas outras coisas nas quais não expliquei, mas isso deve dar uma base para você.

## Cuidado
Lembre-se de que este é apenas um exemplo, não use-o para fins maliciosos ou anti-éticos, mas sim, para aprender e adquirir conhecimento. Obrigado!

## Vídeo exemplar

https://github.com/elDimasX/Rootkit/assets/51800283/ced3011c-e3aa-438d-bfdf-ed81a0f841a5



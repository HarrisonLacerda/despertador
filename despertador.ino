// PROJETO CRIADO PELO SITE MUNDO PROJETADO
// Acesse: www.mundoprojetado.com.br
// Versão 3.2 de um despertador criado usando um shield LCD16x2 e um buzzer
// Atualizações:
//  Versão 3.2: verificação de mes e ano bissexto na configuração do dia da data
//              fornecido pelo leitor: Harrison Lucas Paula Lacerda
//  Versão 3.1: adição de vários alarmes configuráveis via código
//  Versão 3.0: adição da configuração da data e pequenos ajustes no código
//  Versão 2.0: adição da data na tela de horas e reescrita do código de uma melhor forma
//  Versão 1.0: só pode configurar um despertador que fica constante a partir do momento que é ligado
// ----------------------------------------------------------------------------------------------------

// ----- Bibliotecas -----
#include <TimeLib.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


// ----- Definições -----
// Pino do Buzzer
#define BUZZER 13

// Condições dos botões apertados
#define BOTAO_CIMA    analogRead(A0) <= 43 && analogRead(A0) >= 42
#define BOTAO_BAIXO   analogRead(A0) <= 57 && analogRead(A0) >= 58
#define BOTAO_SELECT  analogRead(A0) <= 66 && analogRead(A0) >= 66
#define BOTAO_DIR     analogRead(A0) <= 20
#define BOTAO_ESQ     analogRead(A0) <= 63 && analogRead(A0) >= 62

// Delay (em milisegundos) após pressionar botões
#define DELAY_BOT_SLOW    300
#define DELAY_BOT_FAST    100

// ----- Variaveis globais -----

// Simbolo da seta na configuraçao
byte SIMBOLO_SETA[8] = {
  B00100,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

// Simbolo do relogio
byte SIMBOLO_RELOGIO[8] = {
  B00000,
  B10001,
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000,
};

// Musica do despertador
const char* MUSICA[] = {"Re", "Mi", "Fa", "Sol", "Do", "La", "Fa", "Do", "Fim"};
const int DURACAO[] = {500, 500, 500, 500, 500, 500, 500, 500};


// Frases do menu
const String titulo_menu[4] = {"Mostrar horas", "Definir alarme", "Mudar as horas", "Mudar data"};

// Estados do programa em relação aos menus
enum {
  MENU_MAIN = 0,
  MENU_HORAS,
  MENU_ALARME,
  MENU_CONFIG_HORAS,
  MENU_CONFIG_DATA
}menu_atual;

// Variaveis auxiliares dos menus
uint8_t d1, d2, pos = 0;
uint16_t d3; // É de 16 bits, pois pode armazenar o ano da data

// Variaveis para controle do alarme
bool parou_alarme = false, alarme_ativado = true;

// Variavel dos alarmes
// O primeiro da lista é o configurável pelo display
// Adicione seu alarme mantendo o formato {XX,XX},
unsigned char alarmes[][2] = {{6, 0},
                              {8, 0},
                              {10, 0}
                              };

// ----- Prototipo das funções -----
void verificar_alarme(void);
void tocar(char* mus[], int tempo[]);
String get_hour(void);
String get_date(void) ;

void setup()
{
  // Inicia o LCD e cria os símbolos
  lcd.begin(16, 2);
  lcd.createChar(1, SIMBOLO_SETA);
  lcd.createChar(2, SIMBOLO_RELOGIO);

  // Define hora/min/segundo - dia/mes/ano
  setTime(12, 00, 0, 23, 1, 2021);

  // Configura o pino do buzzer
  pinMode(BUZZER, OUTPUT);
}

void loop()
{
  // Verifica em qual menu está
  switch (menu_atual)
  {
    // Menu principal
    case MENU_MAIN:
      // Escreve os títulos
      lcd.setCursor(0, 0);
      lcd.print(titulo_menu[pos]);
      lcd.setCursor(0, 1);
      if (pos < 3)
      {
        lcd.print(titulo_menu[pos + 1]);
      }
 
      // Simbolo do relogio caso o despertador esteja ativo
      if (!parou_alarme && alarme_ativado)
      {
        lcd.setCursor(15, 0);
        lcd.write(2);
      }
   
      // ----- Testa os botoes -----
      if (BOTAO_CIMA && pos > 0)
      {
        pos--;  
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_BAIXO && pos < 3)
      {
        pos++;  
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_SELECT)
      {
        // Verifica em qual menu deve entrar
        switch (pos)
        {
          case 0:
            menu_atual = MENU_HORAS;
            break;
          case 1:
            menu_atual = MENU_ALARME;
            pos = 0;
            d1 = alarmes[0][0];
            d2 = alarmes[0][1];
            d3 = alarme_ativado;
            break;
          case 2:
            menu_atual = MENU_CONFIG_HORAS;
            pos = 0;
            d1 = hour();
            d2 = minute();
            break;
          case 3:
            menu_atual = MENU_CONFIG_DATA;
            pos = 0;
            d1 = day();
            d2 = month();
            d3 = year();
        }
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }
     
      break;
    // ----- MENU_MAIN -----


    case MENU_HORAS:
      lcd.setCursor(3, 0);
      lcd.print(get_hour());
      lcd.setCursor(3, 1);
      lcd.print(get_date());
     
      // Simbolo do relogio caso o despertador esteja ativo
      if (!parou_alarme && alarme_ativado)
      {
        lcd.setCursor(15, 0);
        lcd.write(2);
      }
     
      if (BOTAO_SELECT)
      {
        menu_atual = MENU_MAIN;
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }
     
      break;
    // ----- MENU_HORAS -----
   

    case MENU_ALARME:
      // Escreve as horas do alarme
      lcd.setCursor(0, 0);
      if (d1 < 10)
      {
        lcd.print("0");
      }
      lcd.print(d1);
      lcd.print(":");
     
      if (d2 < 10)
      {
        lcd.print("0");
      }
      lcd.print(d2);

      // Informa se está ativo ou não
      lcd.setCursor(7, 0);
      if (d3)
      {
        lcd.print("Ativado");
      }
      else
      {
        lcd.setCursor(6, 0);
        lcd.print("Desativado");
      }

      // Desenha a seta
      switch (pos)
      {
        case 0:
          lcd.setCursor(1, 1);
          break;
        case 1:
          lcd.setCursor(3, 1);
          break;
        case 2:
          lcd.setCursor(10, 1);
          break;
      }
      lcd.write(1);
     
      // ----- Testa os botoes -----
      if (BOTAO_CIMA)
      {
        if (pos == 0 && d1 < 23)
        {
          d1 ++;
        }
        else if (pos == 1 && d2 < 59)
        {
          d2 ++;
        }
        else if (pos == 2 && d3 < 1)
        {
          d3 ++;
          lcd.setCursor(6, 0);
          lcd.print(" ");
          lcd.setCursor(14, 0);
          lcd.print("  ");
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_BAIXO)
      {
        if (pos == 0 && d1 > 0)
        {
          d1 --;
        }
        else if (pos == 1 && d2 > 0)
        {
          d2 --;
        }
        else if (pos == 2 && d3 > 0)
        {
          d3 --;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_DIR && pos < 2)
      {
          pos++;
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_ESQ && pos > 0)
      {
          pos--;  
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_SELECT)
      {
        alarmes[0][0] = d1;
        alarmes[0][1] = d2;
        alarme_ativado = d3;
       
        menu_atual = MENU_MAIN;
        pos = 0;
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }

      break;
      // ----- MENU_ALARME -----


    case MENU_CONFIG_HORAS:
      // Escreve as horas
      lcd.setCursor(5, 0);
      if (d1 < 10)
      {
        lcd.print("0");
      }    
      lcd.print(d1);
      lcd.print(":");
     
      if (d2 < 10)
      {
        lcd.print("0");
      }
      lcd.print(d2);

      // Desenha a seta
      lcd.setCursor(6 + pos*2, 1);  
      lcd.write(1);
     
      // ----- Testa os botoes -----
      if (BOTAO_CIMA)
      {
        if (pos == 0 && d1 < 23)
        {
          d1 ++;
        }
        else if (pos == 1 && d2 < 59)
        {
          d2 ++;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_BAIXO)
      {
        if (pos == 0 && d1 > 0)
        {
          d1 --;
        }
        else if (pos == 1 && d2 > 0)
        {
          d2 --;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_DIR && pos < 1)
      {
          pos++;  
          lcd.setCursor(6, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_ESQ && pos > 0)
      {
          pos--;  
          lcd.setCursor(8, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_SELECT)
      {
        setTime(d1, d2, 0, day(), month(), year());
        menu_atual = MENU_MAIN;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }

      break;
      // ----- MENU_CONFIG_HORAS -----
     
 
    case MENU_CONFIG_DATA:
      // Escreve a data
      lcd.setCursor(3, 0);
      if (d1 < 10)
      {
        lcd.print("0");
      }    
      lcd.print(d1);
      lcd.print("/");
     
      if (d2 < 10)
      {
        lcd.print("0");
      }
      lcd.print(d2);

      lcd.print("/");
      lcd.print(d3);

      // Desenha a seta
      lcd.setCursor(4 + pos*3, 1);
      lcd.write(1);
     
      // ----- Testa os botoes -----
      if (BOTAO_CIMA)
      {
        uint8_t aux;
       
        // Restringe o valor do dia de acordo com o mês e ano (se for bissexto)
        if(d2 == 1 || d2 == 3 || d2 == 5 || d2 == 7 || d2 == 8 || d2 == 10 || d2 == 12)
        {
          aux = 31;
        }
        else if(d2 == 2)
        {
          if(d3%4 == 0)
          {
            aux = 29;
          }
          else
          {
            aux = 28;
          }
        }
        else // Condições faltantes: mes 4, 6, 9 ou 11
        {
          aux = 30;
        }
       
        if (pos == 0 && d1 < aux)
        {
          d1++;
        }
        else if (pos == 1 && d2 < 12)
        {
          d2++;
        }
        else if(pos == 2 && d3 < 9999)
        {
          d3++;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_BAIXO)
      {
        if (pos == 0 && d1 > 1)
        {
          d1--;
        }
        else if (pos == 1 && d2 > 1)
        {
          d2--;
        }
        else if(pos == 2 && d3 > 0)
        {
          d3--;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (BOTAO_DIR && pos < 2)
      {
          pos++;  
          lcd.setCursor(4, 1);
          lcd.print("       ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_ESQ && pos > 0)
      {
          pos--;  
          lcd.setCursor(4, 1);
          lcd.print("       ");
          delay(DELAY_BOT_SLOW);
      }
      else if (BOTAO_SELECT)
      {
        setTime(hour(), minute(), second(), d1, d2, d3);
       
        menu_atual = MENU_MAIN;
        pos = 0;
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }

      break;
      // ----- MENU_CONFIG_HORAS -----
  }

  // Verifica se esta na hora de despertar
  verificar_alarme();
}


/*
 * Verifica se está na hora de algum alarme tocar
 */
void verificar_alarme(void)
{
  unsigned char i = 0;
  for(i = 0; i < sizeof(alarmes)/2; i++)
  {
    if (hour() == alarmes[i][0] && minute() == alarmes[i][1] && !parou_alarme && alarme_ativado)
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Segure select");
      lcd.setCursor(3, 1);
      lcd.print("para parar");
     
      while (!parou_alarme)
      {
        tocar(MUSICA, DURACAO);
        if (BOTAO_SELECT)
        {
          parou_alarme = true;
        }
        delay(200);
      }
     
      lcd.clear();
    }
  }
 
  // So reinicia a variável quando os segundos forem menor que 2 (próximo minuto)
  if (parou_alarme && second() <= 2)
  {
    parou_alarme = false;
  }
}


/*
 * Toca uma musica de acordo com o parâmetro
 */
void tocar(char* mus[], int tempo[])
{
  int tom = 0;
  for (int i = 0; mus[i] != "Fim"; i++)
  {
    if (mus[i] == "Do") tom = 262;
    if (mus[i] == "Re") tom = 294;
    if (mus[i] == "Mi") tom = 330;
    if (mus[i] == "Fa") tom = 349;
    if (mus[i] == "Sol") tom = 392;
    if (mus[i] == "La") tom = 440;
    if (mus[i] == "Si") tom = 494;
    if (mus[i] == "Do#") tom = 528;
    if (mus[i] == "Re#") tom = 622;
    if (mus[i] == "Fa#") tom = 370;
    if (mus[i] == "Sol#") tom = 415;
    if (mus[i] == "La#") tom = 466;
    if (mus[i] == "Pausa") tom = 0;
   
    tone(BUZZER, tom, tempo[i]);
    delay(tempo[i]);

    // CONDIÇAO DO DESPERTADOR - Não tem relação com essa função
    if (BOTAO_SELECT)
    {
      parou_alarme = true;  
      break;
    }
  }
}


/*
 * Retorna uma string com o horário atual no formato:
 * HH:MM:SS
 */
String get_hour(void)
{
  String horario = "";
  unsigned char i, aux;
 
  for (i=0; i < 3; i++)
  {
    if(i == 0)
    {
      aux = hour();
    }
    else if(i == 1)
    {
      aux = minute();
    }
    else if(i == 2)
    {
      aux = second();
    }
   
    if(aux < 10)
    {
      horario += "0";
    }
    horario += aux;

    if(i != 2)
    {
      horario += ":";
    }
  }
   
  return horario;
}

/*
 * Retorna uma string com a data no formato:
 * DD/MM/AAAA
 */
String get_date(void)
{
  String data = "";
  unsigned int i, aux;
 
  for (i=0; i < 3; i++)
  {
    if(i == 0)
    {
      aux = day();
    }
    else if(i == 1)
    {
      aux = month();
    }
    else if(i == 2)
    {
      aux = year();
    }
   
    if(aux < 10)
    {
      data += "0";
    }
    data += aux;

    if(i != 2)
    {
      data += "/";
    }
  }
   
  return data;
}
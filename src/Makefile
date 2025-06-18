# Nome do compilador C
CC = gcc

# Flags de compilação: -Wall (todos os avisos), -Wextra (mais avisos),
# -std=c99 (padrão C99), -g (informações de debug)
CFLAGS = -Wall -Wextra -std=c99 -g

# Nome do executável alvo
TARGET = escalona

# Lista de todos os arquivos fonte .c
SOURCES = escalona.c equivalencia.c

# Lista de arquivos objeto .o (derivada automaticamente da lista de fontes)
OBJECTS = $(SOURCES:.c=.o)

# Regra padrão: compila o alvo principal. É a primeira regra, então é a default.
all: $(TARGET)

# Regra de Linkagem: Como criar o executável final a partir dos arquivos objeto.
# O executável $(TARGET) depende de todos os arquivos objeto $(OBJECTS).
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Regra de Compilação Genérica: Como criar um arquivo .o a partir de um .c
# Esta regra ensina ao 'make' como compilar qualquer arquivo .c do projeto.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependências Específicas:
# Aqui dizemos ao 'make' que se um arquivo .h mudar, os arquivos .o que o incluem
# precisam ser recompilados.
escalona.o: escalona.c equivalencia.h
equivalencia.o: equivalencia.c equivalencia.h

# Regra para limpar os arquivos gerados (o executável e os objetos)
clean:
	rm -f $(TARGET) $(OBJECTS)
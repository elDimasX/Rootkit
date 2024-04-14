






PFLT_PREOP_CALLBACK_STATUS MiniFiltroPreCreate(
	OUT PFLT_CALLBACK_DATA Data,
	IN PCFLT_RELATED_OBJECTS Objeto,
	IN PVOID Contexto
)
{
	UNREFERENCED_PARAMETER(Objeto);
	UNREFERENCED_PARAMETER(Contexto);


	NTSTATUS RetornarStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

	__try {

		PFLT_FILE_NAME_INFORMATION Info;
		NTSTATUS Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &Info);

		if (!NT_SUCCESS(Status))
			return RetornarStatus;

		Status = FltParseFileNameInformation(Info);

		if (!NT_SUCCESS(Status))
		{
			FltReleaseFileNameInformation(Info);
			return RetornarStatus;
		}

		// Deixe em maiusculo
		_wcsupr(Info->Name.Buffer);


		if (wcsstr(Info->Name.Buffer, L"\\DRIVERS\\ROOTKIT.SYS"))
		{

			ExplorerProcesso(PsGetCurrentProcess());

			KdPrint(("Minifiltro bloqueando operação: %wZ", Info->Name));

			Data->IoStatus.Status = STATUS_ACCESS_DENIED;
			Data->IoStatus.Information = 0;

			RetornarStatus = FLT_PREOP_COMPLETE;
		}

		FltReleaseFileNameInformation(Info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

	}

	return RetornarStatus;
}


// Filtro
PFLT_FILTER Filtro = NULL;

// Chamdas que queremos registrar
const FLT_OPERATION_REGISTRATION Chamadas[] =
{
	
	{IRP_MJ_CREATE, 0, MiniFiltroPreCreate, NULL},

	// IRP_MJ_CLOSE dá mais proteção para os arquivos, pois bloqueia e diz que existe uma alça, o que faz impossível de deletar enquanto o minifiltro estiver rodando (diferente do IRP_MJ_CREATE)
	{IRP_MJ_CLOSE, 0, MiniFiltroPreCreate, NULL},

	{IRP_MJ_OPERATION_END} // Fim

};

// Apenas definição
NTSTATUS Descarregar(
	IN FLT_FILTER_UNLOAD_FLAGS Flags
);

// Estrutura para criar o filtro
const FLT_REGISTRATION RegistroMinifiltro =
{
	sizeof(FLT_REGISTRATION), // Tamanho do nosso registro
	FLT_REGISTRATION_VERSION, // Nossa versão do registro
	FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP,
	NULL,
	Chamadas,			// Chamadas
	/*Descarregar*/NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

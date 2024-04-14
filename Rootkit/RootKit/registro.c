


LARGE_INTEGER RegistroRegedit = { 0 };


NTKERNELAPI NTSTATUS ObQueryNameString(
	IN PVOID Objeto,
	OUT POBJECT_NAME_INFORMATION ObjetoNomeInfo,
	IN ULONG Tamanho,
	OUT PULONG RetornarTamanho
);


/// <summary>
/// Obt�m o nome completo da opera��o no registro, e retorna no argumento 'Registro'
/// </summary>
/// 
/// <param name="Registro">UNICODE_STRING, que vai conter o nome</param>
/// <param name="ObjetoRegistro">Objeto atual da opera��o do registro</param>
/// 
/// <returns>Retorna um BOOL para saber se conseguiu ou n�o obter o nome do registro</returns>
BOOLEAN ObterNomeCompletoDoRegistro(
	IN PUNICODE_STRING Registro,
	IN PVOID ObjetoRegistro
)
{

	BOOLEAN Nome = FALSE;
	BOOLEAN Parcial = FALSE;

	NTSTATUS Status = STATUS_SUCCESS;

	// Se n�o for um local v�lido no endere�o
	if (!MmIsAddressValid(ObjetoRegistro) || (ObjetoRegistro) == NULL)
	{
		return FALSE;
	}

	// Se for falso
	if (Nome == FALSE)
	{

		ULONG TamanhoRetornar;
		PUNICODE_STRING ObjetoNome = NULL;

		// Se conseguir obter o nome (falhar�, e ent�o, obtemos o tamanho para alocarmos corretamente no TamanhoRetornar)
		Status = ObQueryNameString(
			ObjetoRegistro,
			(POBJECT_NAME_INFORMATION)ObjetoNome,
			0, // N�o sabemos o valor exato, ent�o, obteremos ele no 'TamanhoRetornar'
			&TamanhoRetornar
		);
		
		// Se o tamanho da estrutura que queremos usar para armazenar o nome do objeto n�o � suficiente.
		if (Status == STATUS_INFO_LENGTH_MISMATCH)
		{

			// Aloque com o espa�o que foi nos dado no 'TamanhoRetornar'
			ObjetoNome = ExAllocatePoolWithTag(PagedPool, TamanhoRetornar, 'kffe');

			// Se conseguiu alocar
			if (ObjetoNome)
			{
				Status = ObQueryNameString(
					ObjetoRegistro,
					(POBJECT_NAME_INFORMATION)ObjetoNome,
					TamanhoRetornar, // Tamanho exato
					&TamanhoRetornar
				);

				if (NT_SUCCESS(Status))
				{

					// Copie para o Registro, o ObjetoNome. J� que obtemos ele no ObQueryNameString
					RtlCopyUnicodeString(Registro, ObjetoNome);

					// Conseguimos obter o nome
					Nome = TRUE;
				}

				ExFreePoolWithTag(ObjetoNome, 'kffe');

			}

		}

	}

	return Nome;
}


/// <summary>
/// Monitora as opera��es de registro. E se algum evento conter o nome 'Rootkit', ele bloqueia
/// </summary>
/// 
/// <param name="Contexto">Contexto</param>
/// <param name="Argumento1">Argumento</param>
/// <param name="Argumento2">Argumento</param>
/// 
/// <returns>Retorna um NTSTATUS</returns>
NTSTATUS OperacaoRegistro(
	IN PVOID Contexto,
	IN PVOID Argumento1,
	IN PVOID Argumento2
)
{

	NTSTATUS RetornarStatus = STATUS_SUCCESS;

	__try {

		REG_NOTIFY_CLASS Classe = (REG_NOTIFY_CLASS)(ULONG_PTR)Argumento1;

		//
		UNICODE_STRING LocalRegistro;
		LocalRegistro.Length = 0;
		LocalRegistro.MaximumLength = 1024 * sizeof(WCHAR);

		LocalRegistro.Buffer = ExAllocatePoolWithTag(PagedPool, LocalRegistro.MaximumLength, 'effg');

		// Se n�o alocar
		if (LocalRegistro.Buffer == NULL)
		{
			return STATUS_MEMORY_NOT_ALLOCATED;
		}

		if (
			// Deletando um valor de uma chave
			RegNtPreDeleteValueKey == Classe ||

			// Alterando uma ACL
			RegNtPreSetKeySecurity == Classe ||

			// Substituir
			RegNtPreReplaceKey == Classe ||

			// Alterar informa��o
			RegNtPreSetInformationKey == Classe ||

			// Fechar al�a
			RegNtPreKeyHandleClose == Classe ||

			// Flush
			RegNtPreFlushKey == Classe ||

			// Proibido
			/*RegNtPreOpenKeyEx == Class ||*/

			// Restaurar
			RegNtPreRestoreKey == Classe ||

			// Descarregar chave (muita prote��o)
			RegNtPreUnLoadKey == Classe ||

			// Renomear
			RegNtRenameKey == Classe ||

			// Deletando uma chave (pasta)
			RegNtPreDeleteKey == Classe ||

			// Setando um valor em uma chave
			RegNtPreSetValueKey == Classe ||

			// Nova chave criada
			RegNtPreCreateKey == Classe ||

			// Renomeando uma chave
			RegNtPreRenameKey == Classe 




			/*
			// Pra fuder com tudo
			|| RegNtPreQueryKeyName == Classe ||
			RegNtPreQueryKey == Classe ||
			RegNtPreQueryKeySecurity == Classe ||
			RegNtPreQueryValueKey == Classe ||
			RegNtPreQueryMultipleValueKey == Classe ||
			RegNtPreEnumerateKey == Classe ||
			RegNtPreEnumerateValueKey == Classe ||
			RegNtPreLoadKey == Classe ||
			RegNtPreOpenKey == Classe
			*/
			)
		{

			// Tente obter o local do registro
			BOOLEAN Obteu = ObterNomeCompletoDoRegistro(
				&LocalRegistro,
				((PREG_SET_VALUE_KEY_INFORMATION)Argumento2)->Object
			);

			// Se conseguiu obter, e colar o nome no 'LocalRegistro'
			if (Obteu == TRUE)
			{

				// Deixe em minusculo
				_wcslwr(LocalRegistro.Buffer);

				if (wcsstr(LocalRegistro.Buffer, L"rootkit") ||


					// Impede que desative o bcdedit, quando ele estiver ativado
					wcsstr(LocalRegistro.Buffer, L"elements\\16000049"))
				{
					if (ProtegerRegistro == TRUE)
					{
						KdPrint(("Tentativa de evento ao registro bloqueado %wS\n", LocalRegistro.Buffer));
						
						// Finalize processos que queiram modificar os valores
						//ExplorerProcesso(PsGetCurrentProcess());

						// Acesso negado
						RetornarStatus = STATUS_ACCESS_DENIED;
					}
				}
			}
		}

		if (LocalRegistro.Buffer != NULL)
			ExFreePoolWithTag(LocalRegistro.Buffer, 'effg');

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

	}

	return RetornarStatus;

}

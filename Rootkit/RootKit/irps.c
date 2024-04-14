



NTSTATUS Criado(
	IN PDEVICE_OBJECT Objeto,
	IN PIRP Irp
)
{


	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Mensagem(
	IN PDEVICE_OBJECT Objeto,
	IN PIRP Irp
)
{

	PIO_STACK_LOCATION Io = IoGetCurrentIrpStackLocation(Irp);
	CHAR* RetornarMensagem = "FDS USER-MODE";

	// Mensagem do user-mode
	PCHAR MensagemUsuario = (PCHAR)Irp->AssociatedIrp.SystemBuffer;

	// Verifique o tamanho da mensagem do usuário
	size_t MensagemUsuarioSize = Io->Parameters.DeviceIoControl.InputBufferLength;
	
	// Se conter uma mensagem
	if (MensagemUsuarioSize > 0)
	{
		// Termine com NULO
		MensagemUsuario[MensagemUsuarioSize - 1] = '\0';

		if (strcmp(MensagemUsuario, "Alterar proteção de registro") == 0)
		{
			ProtegerRegistro = !ProtegerRegistro;
			RetornarMensagem = "Ok, alterado!";
		}
	}

	int MaximoCopiar = strlen(RetornarMensagem) + 1;

	// Copie para o user-mode
	RtlCopyMemory(
		Irp->AssociatedIrp.SystemBuffer,
		RetornarMensagem,
		MaximoCopiar
	);

	// Sucesso
	Irp->IoStatus.Status = STATUS_SUCCESS;

	// Máximo de caracteres para nós retornar ao user-mode
	Irp->IoStatus.Information = MaximoCopiar;

	// Complete a requisição
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;

}

NTSTATUS Fechado(
	IN PDEVICE_OBJECT Objeto,
	IN PIRP Irp
)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}



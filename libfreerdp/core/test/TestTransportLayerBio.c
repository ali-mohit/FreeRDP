/* FreeRDP: Licensed under the Apache License, Version 2.0. */
#include <freerdp/config.h>
#include <stdio.h>
#include <openssl/bio.h>
#include <freerdp/transport_io.h>
#include "../transport.h"

int TestTransportLayerBio(int argc, char* argv[])
{
	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);
	int result = -1;
	BIO* bio = BIO_new(BIO_s_transport_layer());
	if (!bio)
		return result;
	rdpTransportLayer* layer = transport_layer_new(nullptr, 0);
	if (!layer)
		goto out;
	BIO_set_data(bio, layer);
#ifdef BIO_CTRL_GET_KTLS_SEND
	if (BIO_ctrl(bio, BIO_CTRL_GET_KTLS_SEND, 0, nullptr) != 0)
		goto out;
#endif
#ifdef BIO_CTRL_GET_KTLS_RECV
	if (BIO_ctrl(bio, BIO_CTRL_GET_KTLS_RECV, 0, nullptr) != 0)
		goto out;
#endif
	/* Capability queries must not change the ordinary BIO lifecycle. */
	if ((BIO_ctrl(bio, BIO_CTRL_FLUSH, 0, nullptr) != 1) ||
	    (BIO_ctrl(bio, BIO_CTRL_SET_CLOSE, BIO_NOCLOSE, nullptr) != 1) ||
	    (BIO_ctrl(bio, BIO_CTRL_GET_CLOSE, 0, nullptr) != BIO_NOCLOSE))
		goto out;
	result = 0;
out:
	if (result != 0)
		(void)fprintf(stderr, "Transport BIO capability or lifecycle control failed\n");
	BIO_free(bio);
	return result;
}

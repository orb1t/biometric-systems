#include <TutorialUtils.h>

#ifdef N_MAC_OSX_FRAMEWORKS
	#include <NCore/NCore.h>
	#include <NBiometricClient/NBiometricClient.h>
	#include <NBiometrics/NBiometrics.h>
	#include <NMedia/NMedia.h>
#else
	#include <NBiometrics.h>
	#include <NBiometricClient.h>
	#include <NCore.h>
	#include <NMedia.h>
#endif

const NChar title[] = N_T("CreateFaceTemplateOnServer");
const NChar description[] = N_T("Demonstrates how to create face template from image on server.");
const NChar version[] = N_T("9.0.0.0");
const NChar copyright[] = N_T("Copyright (C) 2015-2017 Neurotechnology");

const NChar * defaultServerIp = N_T("127.0.0.1");
const NInt defaultAdminPort = 24932;
const NInt defaultPort = 25452;

int usage()
{
	printf(N_T("usage:\n"));
	printf(N_T("\t%s [server] [port] [input image] [output template]\n"), title);
	printf(N_T("\n"));
	printf(N_T("\t[server] - matching server address.\n"));
	printf(N_T("\t[port] - matching port address (if server address specified - port is optional).\n"));
	printf(N_T("\t[image]   - image filename to extract.\n"));
	printf(N_T("\t[template] - filename to store extracted features.\n"));
	printf(N_T("\n\nexample:\n"));
	printf(N_T("\t%s 127.0.0.1 image.jpg template.dat\n"), title);
	printf(N_T("\t%s 127.0.0.1 24932 image.jpg template.dat\n"), title);
	return 1;
}

int main(int argc, NChar **argv)
{
	HNBiometricClient hBiometricClient = NULL;
	HNSubject hSubject = NULL;
	HNFace hFace = NULL;
	HNClusterBiometricConnection hClusterBiometricConnection = NULL;
	HNBuffer hBuffer = NULL;
	HNString hBiometricStatus = NULL;

	NChar * hFaceInput = NULL;
	NChar * hTemplate = NULL;
	NResult result = N_OK;
	NBiometricStatus biometricStatus = nbsNone;
	const NChar * szBiometricStatus = NULL;
	const NChar * serverIp = defaultServerIp;
	NInt adminPort = defaultAdminPort;

	OnStart(title, description, version, copyright, argc, argv);

	if (argc < 4)
	{
		OnExit();
		return usage();
	}

	// create biometric client
	result = NBiometricClientCreate(&hBiometricClient);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NBiometricClientCreate() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// perform all biometric operations on remote server only
	result = NBiometricClientSetLocalOperations(hBiometricClient, nboNone);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NBiometricClientSetLocalOperations() failed (result = %d)!"), result);
		goto FINALLY;
	}

	if (argc == 4)
	{
		serverIp = argv[1];
		hFaceInput = argv[2];
		hTemplate = argv[3];
	}

	if (argc == 5)
	{
		serverIp = argv[1];
		adminPort = atoi(argv[2]);
		hFaceInput = argv[3];
		hTemplate = argv[4];
	}
	
	// add remote connection to cluster
	result = NBiometricClientAddRemoteConnectionToCluster(hBiometricClient, serverIp, defaultPort, adminPort, &hClusterBiometricConnection);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NBiometricClientAddRemoteConnectionToClusterN() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// create subject
	result = NSubjectCreate(&hSubject);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NSubjectCreate() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// create face for the subject
	result = NFaceCreate(&hFace);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NFaceCreate() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// read and set the image for the face
	result = NFileReadAllBytesCN(hFaceInput, &hBuffer);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NFileReadAllBytesCN() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NBiometricSetSampleBuffer(hFace, hBuffer);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NBiometricSetSampleBuffer() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSet(NULL, &hBuffer);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NObjectSet() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// set the face for the subject
	result = NSubjectAddFace(hSubject, hFace, NULL);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NSubjectAddFace() failed (result = %d)!"), result);
		goto FINALLY;
	}

	{
		NTemplateSize templateSize = ntsLarge;

		// set template size to large
		result = NObjectSetPropertyP(hBiometricClient, N_T("Faces.TemplateSize"), N_TYPE_OF(NTemplateSize), naNone, &templateSize, sizeof(templateSize), 1, NTrue);
		if (NFailed(result))
		{
			result = PrintErrorMsgWithLastError(N_T("NObjectSetProperty() failed (result = %d)!"), result);
			goto FINALLY;
		}
	}

	// create the template
	result = NBiometricEngineCreateTemplate(hBiometricClient, hSubject, &biometricStatus);
	if (NFailed(result))
	{
		result = PrintErrorMsgWithLastError(N_T("NBiometricEngineCreateTemplate() failed (result = %d)!"), result);
		goto FINALLY;
	}

	if (biometricStatus == nbsOk)
	{
		printf(N_T("template extracted\n"));

		// retrieve the template from subject
		result = NSubjectGetTemplateBuffer(hSubject, &hBuffer);
		if (NFailed(result))
		{
			result = PrintErrorMsgWithLastError(N_T("NSubjectGetTemplateBuffer() failed (result = %d)!"), result);
			goto FINALLY;
		}

		result = NFileWriteAllBytesCN(hTemplate, hBuffer);
		if (NFailed(result))
		{
			result = PrintErrorMsgWithLastError(N_T("failed to write template to file (result = %d)!"), result);
			goto FINALLY;
		}

		printf(N_T("template saved successfully\n"));
	}
	else
	{
		// Retrieve biometric status
		result = NEnumToStringP(N_TYPE_OF(NBiometricStatus), biometricStatus, NULL, &hBiometricStatus);
		if (NFailed(result))
		{
			result = PrintErrorMsgWithLastError(N_T("NEnumToStringP() failed (result = %d)!"), result);
			goto FINALLY;
		}

		result = NStringGetBuffer(hBiometricStatus, NULL, &szBiometricStatus);
		if (NFailed(result))
		{
			result = PrintErrorMsgWithLastError(N_T("NStringGetBuffer() failed (result = %d)!"), result);
			goto FINALLY;
		}

		printf(N_T("template extraction failed!\n"));
		printf(N_T("biometric status = %s.\n\n"), szBiometricStatus);

		result = N_E_FAILED;
		goto FINALLY;
	}

	result = N_OK;
FINALLY:
	{
		NResult result2 = NObjectSet(NULL, &hBiometricClient);
		if (NFailed(result2)) PrintErrorMsg(N_T("NObjectSet() failed (result = %d)!"), result2);
		result2 = NObjectSet(NULL, &hSubject);
		if (NFailed(result2)) PrintErrorMsg(N_T("NObjectSet() failed (result = %d)!"), result2);
		result2 = NObjectSet(NULL, &hClusterBiometricConnection);
		if (NFailed(result2)) PrintErrorMsg(N_T("NObjectSet() failed (result = %d)!"), result2);
		result2 = NStringSet(NULL, &hBiometricStatus);
		if (NFailed(result2)) PrintErrorMsg(N_T("NStringSet() failed (result = %d)!"), result2);
		result2 = NObjectSet(NULL, &hFace);
		if (NFailed(result2)) PrintErrorMsg(N_T("NObjectSet() failed (result = %d)!"), result2);
		result2 = NObjectSet(NULL, &hBuffer);
		if (NFailed(result2)) PrintErrorMsg(N_T("NObjectSet() failed (result = %d)!"), result2);
	}
	OnExit();
	return result;
}

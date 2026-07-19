// PlatformToolStubs.cpp
// Headless platform hooks required by command-line tools on macOS.

int PlatformPeekMessage(int *pMsg, int *pWParam, void **ppLParam)
	{
	if (pMsg) *pMsg = 0;
	if (pWParam) *pWParam = 0;
	if (ppLParam) *ppLParam = nullptr;
	return 0;
	}

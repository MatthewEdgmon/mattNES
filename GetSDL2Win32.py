import os
import urllib.request
import zipfile

def DownloadZIP(url, extractedFolderName = None):
    zipFileName = url.rsplit('/', 1)[-1]
    assert zipFileName.endswith('.zip')
    if not extractedFolderName:
        extractedFolderName = zipFileName[:-4]
    if not os.path.exists(extractedFolderName):
        print('Downloading %s' % zipFileName)
        urllib.request.urlretrieve(url, zipFileName)
        zipfile.ZipFile(zipFileName).extractall()
        os.remove(zipFileName)
        assert os.path.exists(extractedFolderName)

thirdPartyFolder = os.path.normpath(os.path.join(__file__, '../3rdParty'))
os.makedirs(thirdPartyFolder, exist_ok=True)
os.chdir(thirdPartyFolder)

# SDL2
DownloadZIP('https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip', 'SDL2-2.0.10')

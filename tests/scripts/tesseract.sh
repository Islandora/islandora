apt-get update
apt-get install build-essential checkinstall 
mkdir ~/tesseract
cd ~/tesseract
wget http://www.leptonica.org/source/leptonica-1.69.tar.gz
tar xf leptonica-1.69.tar.gz && rm -rf leptonica-1.69.tar.gz
cd leptonica-1.69
./configure
make && checkinstall --pkgname=libleptonica --pkgversion="1.69" --backup=no --deldoc=yes --fstrans=no --default
cd ~/tesseract
wget https://tesseract-ocr.googlecode.com/files/tesseract-ocr-3.02.02.tar.gz
tar xf tesseract-ocr-3.02.02.tar.gz && rm -rf tesseract-ocr-3.02.02.tar.gz
cd tesseract-ocr
./autogen.sh
./configure
make && checkinstall --pkgname=tesseract-ocr --pkgversion="3.02.02" --backup=no --deldoc=yes --fstrans=no --default && ldconfig
mkdir ~/tesseract/langs
cd ~/tesseract/langs
wget https://tesseract-ocr.googlecode.com/files/tesseract-ocr-3.02.eng.tar.gz
echo "Extracting language files"
for i in *.tar.gz; do echo -e "\nworking on $i\n"; tar xvzf $i ; done
cp tesseract-ocr/tessdata/* /usr/local/share/tessdata/
echo -e "\ntesseract output:"
tesseract --version && tesseract --list-langs && cd ~ && rm -rf ~/tesseract

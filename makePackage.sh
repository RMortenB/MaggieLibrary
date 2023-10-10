
cd fd
./genfd.sh
cd ..

mkdir package
cp -r samples package/

mkdir package/include
cp -r fd/* package/include
cp include/maggie* package/include



make clean
mkdir package/libs
make
make strip
cp maggie.library package/libs
make clean

cp test/Readme.txt package/

mkdir package/bin

cd package/samples

cd ImmediateDraw
make
make strip
cp ImmediateDraw ../../bin
make clean
cd ..

cd Lighting
make
make strip
cp Lighting ../../bin
make clean
cd ..

cd QuadCube
make
make strip
cp QuadCube ../../bin
make clean
cd ..

cd UserProvided
make
make strip
cp UserDraw ../../bin
make clean
cd ..

cd ZBuffer
make
make strip
cp ZBuffer ../../bin
make clean
cd ..

cd Windowed
make
make strip
cp Windowed ../../bin
make clean
cd ..

cd ../..

cp test/TestTexture.dds package/bin/TestTexture.dds

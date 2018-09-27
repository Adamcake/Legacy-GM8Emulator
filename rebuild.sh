if [ -f "CMakeCache.txt" ]; then
	rm -rf CMakeCache.txt
fi

cmake -DCMAKE_GENERATOR_PLATFORM=x64 .
read -n 1 -s -r -p "Press any key..."

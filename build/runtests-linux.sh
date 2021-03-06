#!/bin/bash
read -p "C++ x86 sample..."
../dist/cpp/sample/x86/Release/xt-sample
read -p "C++ x64 sample..."
../dist/cpp/sample/x64/Release/xt-sample
read -p "Java sample..."
java -jar ../dist/java/sample/target/xt.sample-1.7.jar
read -p ".NET Framework sample..."
mono ../dist/cli/sample/Release/net48/Xt.Sample.exe
read -p ".NET Core sample..."
dotnet ../dist/cli/sample/Release/netcoreapp3.1/Xt.Sample.dll
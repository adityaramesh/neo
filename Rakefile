require 'rake/clean'

cxx       = ENV['CXX']
boost     = ENV['BOOST_INCLUDE_PATH']
ccbase    = ENV['CCBASE_INCLUDE_PATH']
eigen     = ENV['EIGEN3_INCLUDE_PATH']
mpl       = ENV['MPL_INCLUDE_PATH']
langflags = "-std=c++11"
wflags    = "-Wall -Wextra -pedantic -Wno-return-type-c-linkage"
archflags = "-march=native"
incflags  = "-I include -isystem #{boost} -isystem #{ccbase} -isystem #{eigen} -isystem #{mpl}"
ldflags   = ""

if cxx.include? "clang"
	optflags = "-Ofast -fno-fast-math -flto -DNDEBUG -DNEO_NO_DEBUG -ggdb"
	#optflags = "-Ofast -ggdb -DNDEBUG -DNEO_NO_DEBUG"
elsif cxx.include? "g++"
	optflags = "-Ofast -fno-fast-math -flto -fwhole-program"
end

cxxflags = "#{langflags} #{wflags} #{archflags} #{incflags} #{optflags}"
tests    = FileList["test/*.cpp"].map{|f| f.sub("test", "out").ext("run")}

task :default => ["out"] + tests

directory "out"

tests.each do |f|
	src = f.sub("out", "test").ext("cpp")
	file f => [src, "out"] do
		sh "#{cxx} #{cxxflags} -o #{f} #{src} #{ldflags}"
	end
end

task :clobber do
	FileList["out/*.run"].each{|f| File.delete(f)}
end

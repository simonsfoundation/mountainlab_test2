from mdaio import readmda,writemda64

X = readmda("test_firings.mda")

writemda64(X,"test2.mda")
\name{Intervals-package}
\alias{Intervals-package}
\alias{Intervals}
\docType{package}
\title{
Tools for the analysis of two ways designs producing paired comparisons plots
%%~~ package title ~~
}
\description{
Tools for the statistical analysis of Two Ways Experimental Designs with and without replications
%% ~~ A concise (1-5 lines) description of the package ~~
}
\details{
\tabular{ll}{
Package: \tab Intervals\cr
Type: \tab Package\cr
Version: \tab 1.0\cr
Date: \tab 2010-09-13\cr
License: \tab GPL-3\cr
LazyLoad: \tab yes\cr
}
Parametric, Rank-based and Permutation test for two-way analysis of variance and multiple comparisons in post-hoc analysis
%%~~ An overview of how to use the package, including the most important functions ~~
}
\author{
Marco Chiarandini

Maintainer: Marco Chiarandini <marco@imada.sdu.dk>
%%~~ The author and/or maintainer of the package ~~
}
\references{
%%~~ Literature or other references for background information ~~
}
%%~~ Optionally other standard keywords, one per line, from file KEYWORDS in the R documentation directory ~~
\keyword{ package }
\seealso{
%%~~ Optional links to other man pages, e.g. ~~
\code{\link[coin:coin-package]{coin}} ~~
}
\examples{
	S <- cbind(expand.grid(instance=c("inst1","inst2"),algo=c("alg1","alg2"),class=c("boh1"),n=c(1,2,3,4)),res=runif(16,min=1,max=3))
    print(S)
    
    R <- AP.SCI(res~algo*instance,data=S,method="Parametric",test="Tukey")
#   plot.ci(R)
#	intervals(algo~y,R)

    R <- AP.SCI(res~algo+instance,data=S,method="Ranks",test="Conover")
#   plot.ci(R)
##  Not working yet
#	intervals(algo~y,R)
    
	
	
    S <- cbind(expand.grid(instance=c("inst1","inst2"),algo=c("alg1","alg2"),class=c("boh1","boh2"),n=c(1,2)),res=runif(16,min=1,max=3))

    R <- AP.SCI.wrapper.class(res~algo*instance,data=S,method="Parametric",test="Tukey")
#   plot.ci.xclass(R)
	intervals(algo~y|class,R,scales=list(relation="free"),par.strip.text=list(cex=0.6),cex=0.7)
	
   	R <- AP.SCI.wrapper.class(res~algo+instance,data=S,method="Ranks",test="Conover")
   	intervals(algo~y|class,R,scales=list(relation="free"),par.strip.text=list(cex=0.6),cex=0.7)
}

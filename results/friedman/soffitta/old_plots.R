# TODO: Add comment
# 
# Author: marco
###############################################################################



plot.ci <- function(ALL,width,height)
{
	require(lattice)
	#require(Hmisc)
	
	.width <- ifelse(missing(width),4,width)
	.height <- ifelse(missing(height),4,height)
	
	
	ALL$algo <- factor(ALL$algo,levels=levels(ALL$algo)[order(ALL$y)])
	## PS #####################################################################
	trellis.device("postscript",file="plot.ci.eps",onefile=TRUE,horizontal=FALSE,
			width=.width,height=.height)
	
	print(xyplot(algo~Cbind(y,lower,upper),data=ALL,
					pch=3, method="bars",pch.bar=2,lwd=3,
					cex=0.8,
					par.strip.text=list(cex=0.8),
					prepanel = function(x, y, ...) {
						ans <- list()
						ans$xlim <- c(min(attr(x,'other')),max(attr(x,'other')))#c(min(out$lower),max(out$upper))#c(xminim,xmaxim)
						ans
					}, 
					panel=function(x,y,...) {
						print(attr(x,'other'))
						panel.abline(v=attr(x,'other'),lty=1,col="grey85")
						panel.Dotplot(x,y,...)
					},
					par.settings = 
							list(layout.heights = 
											list(top.padding = 0.47, 
													main.key.padding = 1)
							),
					scales = list(
							##x=list(at=c(1:nlevels(OUT$algo)),labels=c(1:nlevels(OUT$algo)),alternating=c(1,1,1,1),relation="free"),
							x=list(relation="free",cex=0.8,tck=0.15),
							y=list(
									##labels=expression("GLS", "HEA","ILS","MinConf","Novelty",SA[N6],TS[N1],TS[VLSN],"XRLF"), 
									alternating=c(1,1,1,1),
									cex=0.8,tck=0.15)
					),
					main=list(label="                       All-pairwise comparisons",cex=0.8,font=1),
					xlab="",#list(cex=1,label="average normalised error"),
					ylab="",
					aspect="fill",as.table=TRUE,
					layout=c(1,1)
			)
	)
	dev.off()
}




plot.ci.xclass <- function(ALL,mfrow,width,height)
{
	#require(Hmisc)
	
	if (missing(mfrow))
		.layout <- c(ceiling(sqrt(nlevels(ALL$class))),ceiling(sqrt(nlevels(ALL$class))))
	else
		.layout <- mfrow
	
	.width <- ifelse(missing(width),10,width)
	.height <- ifelse(missing(height),10,height)
	
	Y <- aggregate(ALL$y,list(ALL$algo),sum)
	ALL$algo <- factor(ALL$algo,levels=Y$Group.1[order(Y$x)])
	## PS #####################################################################
	trellis.device("postscript",file="plot.ci.xclass.eps",onefile=TRUE,horizontal=FALSE,
			#paper="a4")
			width=.width, height=.height)
	s <- trellis.par.get("box.dot")
	s$cex <- 0.2
	trellis.par.set("box.dot",s)
	
	s <- trellis.par.get("par.sub.text")
	s$cex <- 0.4
	trellis.par.set("par.sub.text",s)
	
	s <- trellis.par.get("plot.symbol")
	s$cex <- 0.2
	trellis.par.set("plot.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.symbol"),c(1))
	s$cex <- 0.2
	trellis.par.set("superpose.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.line"),c(1))
	s$lwd <- c(2)
	trellis.par.set("superpose.line",s)
	
	print(Dotplot(algo~Cbind(y,lower,upper) |class,data=ALL,
					pch=3, method="bars",pch.bar=2,lwd=3,
					cex=0.3,
					par.strip.text=list(cex=0.6),
					prepanel = function(x, y, ...) {
						ans <- list()
						ans$xlim <- c(min(attr(x,'other')),max(attr(x,'other')))#c(min(out$lower),max(out$upper))#c(xminim,xmaxim)
						ans
					}, 
					panel=function(x,y,...) {
						print(attr(x,'other'))
						panel.abline(v=attr(x,'other'),lty=1,col="grey80")
						panel.Dotplot(x,y,...)
					},
					par.settings = 
							list(layout.heights = 
											list(top.padding = 0.5, 
													main.key.padding = -1.3)
							),
					scales = list(
							x=list(relation="same",alternating=c(1,1,1,1),cex=0.6,tck=0.15),
							y=list(at=c(1:nlevels(ALL$algo)),
									labels=levels(ALL$algo),
									alternating=c(1,1,1,1),cex=0.6,tck=0.15)
					##at=c(1:nlevels(factor(T$name))),labels=levels(factor(T$name)),alternating=c(1,1,1,1))
					),
					##main=list(label="                       All-pairwise comparisons",cex=0.6,font=1),
					main=list(label="                       ",cex=0.6,font=1),
					xlab=list(cex=0.6,label=""),
					ylab="",
					as.table=TRUE,
					layout=.layout#c(ceiling(sqrt(nlevels(ALL$class))),ceiling(sqrt(nlevels(ALL$class))))
			#layout=c(3,1)
			)
	)
	
	dev.off()
}




beamer.ci.xclass <- function(ALL,mfrow,scol,width,height,type="png")
{
	#require(Hmisc)
	
	if (missing(mfrow))
		.layout <- c(ceiling(sqrt(nlevels(ALL$class))),ceiling(sqrt(nlevels(ALL$class))))
	else
		.layout <- mfrow
	
	if (missing(scol))
		.col <- c("black")
	else
		.col <- scol
	
	.width <- ifelse(missing(width),10,width)
	.height <- ifelse(missing(height),10,height)
	
	
	Y <- aggregate(ALL$y,list(ALL$algo),sum)
	ALL$algo <- factor(ALL$algo,levels=Y$Group.1[order(Y$x)])
	
	## PS #####################################################################
	
	if (type=="png")
		trellis.device(type,file="confint.png",theme="col.whitebg",pointsize=12,width=480,height=360)
	else
		trellis.device(type,file=paste("confint.",type,sep=""),theme="col.whitebg",horizontal=FALSE,width=7.5,height=6,family="Helvetica",onefile=TRUE)
	#paste("MCA-",cl,"-",length(unique(res$run)),".eps",sep=""),
	s <- trellis.par.get("box.dot")
	s$cex <- 0.2
	trellis.par.set("box.dot",s)
	
	s <- trellis.par.get("par.sub.text")
	s$cex <- 0.4
	trellis.par.set("par.sub.text",s)
	
	s <- trellis.par.get("plot.symbol")
	s$cex <- 0.2
	trellis.par.set("plot.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.symbol"),c(1))
	s$cex <- 0.2
	trellis.par.set("superpose.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.line"),c(1))
	s$lwd <- c(2)
	trellis.par.set("superpose.line",s)
	
	s <- trellis.par.get("plot.line")
	s$col <- c("blue")
#s$col <- c("red","black","green","blue","khaki2","azure")
	s$lwd <- 2
	trellis.par.set("plot.line",s)
	
	cex.text <- 0.9
	
	print(Dotplot(algo~Cbind(y,lower,upper) |class,data=ALL,
					pch=3, method="bars",pch.bar=2,lwd=3,col="blue",
					cex=0.3,
					par.strip.text=list(cex=cex.text),
					prepanel = function(x, y, ...) {
						ans <- list()
						ans$xlim <- c(min(attr(x,'other')),max(attr(x,'other')))#c(min(out$lower),max(out$upper))#c(xminim,xmaxim)
						ans
					}, 
					panel=function(x,y,...) {
						print(attr(x,'other'))
						panel.abline(v=attr(x,'other'),lty=1,col="grey80")
						panel.Dotplot(x,y,...)
					},
					par.settings = 
							list(layout.heights = 
											list(top.padding = 0.5, 
													main.key.padding = -1.3)
							),
					scales = list(
							x=list(relation="same",alternating=c(1,1,1,1),cex=cex.text,tck=0.15),
							y=list(at=c(1:nlevels(ALL$algo)),
									labels=levels(ALL$algo),
									col=.col,#c("blue","black","blue","black","blue","blue","black","black","black","blue"),
									#col=c("black","blue","black","blue","black","blue","black","blue","black","blue"),
									alternating=c(1,1,1,1),cex=cex.text,tck=0.15)
					##at=c(1:nlevels(factor(T$name))),labels=levels(factor(T$name)),alternating=c(1,1,1,1))
					),
					##main=list(label="                       All-pairwise comparisons",cex=0.6,font=1),
					main=list(label="                       ",cex=cex.text,font=1),
					xlab=list(cex=cex.text,label="Average Rank"),
					ylab="",
					as.table=TRUE,
					#layout=c(ceiling(sqrt(nlevels(ALL$class))),ceiling(sqrt(nlevels(ALL$class))))
					layout=.layout
			)
	)
	
	dev.off()
}




beamer.ci <- function(ALL,scol,width,height,type="png")
{
	require(lattice)
	#require(Hmisc)
	
	if (missing(scol))
		.col <- c("black")
	else
		.col <- scol
	
	.width <- ifelse(missing(width),6,width)
	.height <- ifelse(missing(height),2.4,height)
	
	ALL$algo <- factor(ALL$algo,levels=levels(ALL$algo)[order(ALL$y)])    
	## PS #####################################################################
	
	if (type=="png")
		trellis.device(type,file="confint.png",theme="col.whitebg",pointsize=12,width=480,height=120)
	else
		trellis.device(type,file=paste("confint.",type,sep=""),theme="col.whitebg",horizontal=FALSE,width=.width,height=.height,family="Helvetica",onefile=TRUE)
	#paste("MCA-",cl,"-",length(unique(res$run)),".eps",sep=""),
	cex.text=1
	
	
	s <- trellis.par.get("box.dot")
	s$cex <- 0.2
	trellis.par.set("box.dot",s)
	
	s <- trellis.par.get("par.sub.text")
	s$cex <- 0.4
	trellis.par.set("par.sub.text",s)
	
	s <- trellis.par.get("plot.symbol")
	s$cex <- 0.2
	trellis.par.set("plot.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.symbol"),c(1))
	s$cex <- 0.2
	trellis.par.set("superpose.symbol",s)
	
	s <- Rows(trellis.par.get("superpose.line"),c(1))
	s$lwd <- c(2)
	trellis.par.set("superpose.line",s)
	
	s <- trellis.par.get("plot.line")
	s$col <- c("blue")
#s$col <- c("red","black","green","blue","khaki2","azure")
	s$lwd <- 2
	trellis.par.set("plot.line",s)
	
	cex.text <- 0.9
	
	print(xyplot(algo~Cbind(y,lower,upper),data=ALL,
					pch=3, method="bars",pch.bar=2,lwd=3,col="blue",
					cex=0.8,
					par.strip.text=list(cex=0.8),
					prepanel = function(x, y, ...) {
						ans <- list()
						ans$xlim <- c(min(attr(x,'other')),max(attr(x,'other')))#c(min(out$lower),max(out$upper))#c(xminim,xmaxim)
						ans
					}, 
					panel=function(x,y,...) {
						print(attr(x,'other'))
						panel.abline(v=attr(x,'other'),lty=1,col="grey85")
						panel.Dotplot(x,y,...)
					},
					par.settings = 
							list(layout.heights = 
											list(top.padding = 0.47, 
													main.key.padding = 1)
							),
					scales = list(
							##x=list(at=c(1:nlevels(OUT$algo)),labels=c(1:nlevels(OUT$algo)),alternating=c(1,1,1,1),relation="free"),
							x=list(relation="free",cex=0.8,tck=0.15),
							y=list(
									##labels=expression("GLS", "HEA","ILS","MinConf","Novelty",SA[N6],TS[N1],TS[VLSN],"XRLF"), 
									alternating=c(1,1,1,1),
									col=.col, #c("blue","black","black","blue","blue","blue","blue","blue","black","blue"),
									cex=0.8,tck=0.15)
					),
					main=list(label="",cex=0.8,font=1),
					xlab=list(cex=0.8,label="Average Rank"),
					ylab="",
					aspect="fill",as.table=TRUE,
					layout=c(1,1)
			)
	)
	dev.off()
}



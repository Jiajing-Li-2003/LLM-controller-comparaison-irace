##################################################
# Friedman test for factory experiments
##################################################
# David Garzon-Ramos (dagarzonramos(at)gmail.com)
##################################################

## Rstudio must be opened from the directory of the script

## Cleaning environment

rm(list=ls())

## Set path of files

results_path = "./data/"
plots_path = "./plots/"

## Arguments - Not Used
args = commandArgs(trailingOnly=TRUE)

## Load libraries
library("ggplot2")
library(tikzDevice)
source("./friedman/R/lib_posthoc.R")

## Environment configuration


## Friedman test by methods: Ranking alphas

portfolios <- list()
bestPortfolio <- list()
midPortfolio <- list()
worstPortfolio <- list()

##########################################


for (dataName in c("sim-")) {
  for (methodName in c("au-","ev-","ne-")) {
    for (k in seq(0,5)){
      for (l in seq(0,5)){
        if (k<l){
          
          task = paste("t",k,l, sep = "")
          resultFiles = c()
          
          for (alphaValue in c(2, 4, 5, 6, 8)) {
            filePattern = paste(dataName,methodName,alphaValue,"-",k,l, sep = "")
            missionFiles <- list.files(path=results_path,pattern=filePattern)
            resultFiles <- c(resultFiles,missionFiles)
          }
          
          ## Friedman test
          
          agg_results = data.frame()
          for (i in resultFiles){
            filename = paste(results_path,i,sep = "")
            df = read.csv(filename)
            agg_results <- rbind(agg_results,df)
            rm(df)
          }
          
          agg_results$alg <- as.factor(agg_results$alg)
          agg_results$task <- as.factor(agg_results$task)
          
          R <- AP.SCI(score~alg+task,data=agg_results,method="Ranks",test="Conover")
          
          friedmanMission = paste(dataName,methodName,k,l, sep="")
          friedmanMethodNames = c(levels(R$algo))
          friedmanMethodRanks = c(rank(R$y))
          portfolios[[friedmanMission]] = cbind(friedmanMethodNames,friedmanMethodRanks)
          
          ##
          
          bestPortfolio[[friedmanMission]] <- friedmanMethodNames[match(1,friedmanMethodRanks)]
          midPortfolio[[friedmanMission]] <- friedmanMethodNames[match(3,friedmanMethodRanks)]
          worstPortfolio[[friedmanMission]] <- friedmanMethodNames[match(5,friedmanMethodRanks)]
          
        }
      }
    }
  }
}

worstPortfolio[["sim-au-03"]] = "Au2" # solving ties

# Automode Best - Mid - Worst

rm (agg_results)
rm (filename)
rm (R)

######

simMo =      c("sim-mo-0-01-B.txt", "sim-mo-0-01-R.txt",
               "sim-mo-0-02-B.txt", "sim-mo-0-02-R.txt",
               "sim-mo-0-03-B.txt", "sim-mo-0-03-R.txt",
               "sim-mo-0-04-B.txt", "sim-mo-0-04-R.txt",
               "sim-mo-0-05-B.txt", "sim-mo-0-05-R.txt",
               "sim-mo-0-12-B.txt", "sim-mo-0-12-R.txt",
               "sim-mo-0-13-B.txt", "sim-mo-0-13-R.txt",
               "sim-mo-0-14-B.txt", "sim-mo-0-14-R.txt",
               "sim-mo-0-15-B.txt", "sim-mo-0-15-R.txt",
               "sim-mo-0-23-B.txt", "sim-mo-0-23-R.txt",
               "sim-mo-0-24-B.txt", "sim-mo-0-24-R.txt",
               "sim-mo-0-25-B.txt", "sim-mo-0-25-R.txt",
               "sim-mo-0-34-B.txt", "sim-mo-0-34-R.txt",
               "sim-mo-0-35-B.txt", "sim-mo-0-35-R.txt",
               "sim-mo-0-45-B.txt", "sim-mo-0-45-R.txt") 

######

simHv =      c("sim-hv-0-01-B.txt", "sim-hv-0-01-R.txt",
               "sim-hv-0-02-B.txt", "sim-hv-0-02-R.txt",
               "sim-hv-0-03-B.txt", "sim-hv-0-03-R.txt",
               "sim-hv-0-04-B.txt", "sim-hv-0-04-R.txt",
               "sim-hv-0-05-B.txt", "sim-hv-0-05-R.txt",
               "sim-hv-0-12-B.txt", "sim-hv-0-12-R.txt",
               "sim-hv-0-13-B.txt", "sim-hv-0-13-R.txt",
               "sim-hv-0-14-B.txt", "sim-hv-0-14-R.txt",
               "sim-hv-0-15-B.txt", "sim-hv-0-15-R.txt",
               "sim-hv-0-23-B.txt", "sim-hv-0-23-R.txt",
               "sim-hv-0-24-B.txt", "sim-hv-0-24-R.txt",
               "sim-hv-0-25-B.txt", "sim-hv-0-25-R.txt",
               "sim-hv-0-34-B.txt", "sim-hv-0-34-R.txt",
               "sim-hv-0-35-B.txt", "sim-hv-0-35-R.txt",
               "sim-hv-0-45-B.txt", "sim-hv-0-45-R.txt") 

######

simLn =      c("sim-ln-0-01-B.txt", "sim-ln-0-01-R.txt",
               "sim-ln-0-02-B.txt", "sim-ln-0-02-R.txt",
               "sim-ln-0-03-B.txt", "sim-ln-0-03-R.txt",
               "sim-ln-0-04-B.txt", "sim-ln-0-04-R.txt",
               "sim-ln-0-05-B.txt", "sim-ln-0-05-R.txt",
               "sim-ln-0-12-B.txt", "sim-ln-0-12-R.txt",
               "sim-ln-0-13-B.txt", "sim-ln-0-13-R.txt",
               "sim-ln-0-14-B.txt", "sim-ln-0-14-R.txt",
               "sim-ln-0-15-B.txt", "sim-ln-0-15-R.txt",
               "sim-ln-0-23-B.txt", "sim-ln-0-23-R.txt",
               "sim-ln-0-24-B.txt", "sim-ln-0-24-R.txt",
               "sim-ln-0-25-B.txt", "sim-ln-0-25-R.txt",
               "sim-ln-0-34-B.txt", "sim-ln-0-34-R.txt",
               "sim-ln-0-35-B.txt", "sim-ln-0-35-R.txt",
               "sim-ln-0-45-B.txt", "sim-ln-0-45-R.txt") 

##

bestSimAu = c()
midSimAu = c()
worstSimAu = c() 

bestSimEv = c()
midSimEv = c()
worstSimEv = c()

bestSimNe = c()
midSimNe = c()
worstSimNe = c()


for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      
      alphaBSA = gsub("Au","",bestPortfolio[[paste("sim-au-",k,l, sep = "")]])
      alphaMSA = gsub("Au","",midPortfolio[[paste("sim-au-",k,l, sep = "")]])
      alphaWSA = gsub("Au","",worstPortfolio[[paste("sim-au-",k,l, sep = "")]])
      
      alphaBSE = gsub("Ev","",bestPortfolio[[paste("sim-ev-",k,l, sep = "")]])
      alphaMSE = gsub("Ev","",midPortfolio[[paste("sim-ev-",k,l, sep = "")]])
      alphaWSE = gsub("Ev","",worstPortfolio[[paste("sim-ev-",k,l, sep = "")]])
      
      alphaBSN = gsub("Ne","",bestPortfolio[[paste("sim-ne-",k,l, sep = "")]])
      alphaMSN = gsub("Ne","",midPortfolio[[paste("sim-ne-",k,l, sep = "")]])
      alphaWSN = gsub("Ne","",worstPortfolio[[paste("sim-ne-",k,l, sep = "")]])
      
      bestSimAu = c(bestSimAu, paste("sim-au-",alphaBSA,"-",k,l,"-B.txt",sep=""), paste("sim-au-",alphaBSA,"-",k,l,"-R.txt",sep=""))
      midSimAu = c(midSimAu, paste("sim-au-", alphaMSA ,"-",k,l,"-B.txt",sep=""), paste("sim-au-", alphaMSA ,"-",k,l,"-R.txt",sep=""))
      worstSimAu = c(worstSimAu, paste("sim-au-", alphaWSA ,"-",k,l,"-B.txt",sep=""), paste("sim-au-", alphaWSA ,"-",k,l,"-R.txt",sep=""))
      
      bestSimEv = c(bestSimEv, paste("sim-ev-", alphaBSE ,"-",k,l,"-B.txt",sep=""), paste("sim-ev-", alphaBSE ,"-",k,l,"-R.txt",sep=""))
      midSimEv = c(midSimEv, paste("sim-ev-", alphaMSE ,"-",k,l,"-B.txt",sep=""), paste("sim-ev-", alphaMSE ,"-",k,l,"-R.txt",sep=""))
      worstSimEv = c(worstSimEv, paste("sim-ev-", alphaWSE ,"-",k,l,"-B.txt",sep=""), paste("sim-ev-", alphaWSE ,"-",k,l,"-R.txt",sep=""))
      
      bestSimNe = c(bestSimNe, paste("sim-ne-", alphaBSN ,"-",k,l,"-B.txt",sep=""), paste("sim-ne-", alphaBSN ,"-",k,l,"-R.txt",sep=""))
      midSimNe = c(midSimNe, paste("sim-ne-", alphaMSN ,"-",k,l,"-B.txt",sep=""), paste("sim-ne-", alphaMSN ,"-",k,l,"-R.txt",sep=""))
      worstSimNe = c(worstSimNe, paste("sim-ne-", alphaWSN ,"-",k,l,"-B.txt",sep=""), paste("sim-ne-", alphaWSN ,"-",k,l,"-R.txt",sep=""))
      
    }
  }
}

######

simMoScores = data.frame()
for (i in simMo){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  simMoScores <- rbind(simMoScores,df)
  rm(df)
}

######

simHvScores = data.frame()
for (i in simHv){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  simHvScores <- rbind(simHvScores,df)
  rm(df)
}

######

simLnScores = data.frame()
for (i in simLn){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  simLnScores <- rbind(simLnScores,df)
  rm(df)
}

######

bestSimAuScores = data.frame()
for (i in bestSimAu){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  bestSimAuScores <- rbind(bestSimAuScores,df)
  rm(df)
}
bestSimAuScores$alg = as.factor("AuB")

midSimAuScores = data.frame()
for (i in midSimAu){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  midSimAuScores <- rbind(midSimAuScores,df)
  rm(df)
}
midSimAuScores$alg = as.factor("AuM")

worstSimAuScores = data.frame()
for (i in worstSimAu){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  worstSimAuScores <- rbind(worstSimAuScores,df)
  rm(df)
}
worstSimAuScores$alg = as.factor("AuW")

######

bestSimEvScores = data.frame()
for (i in bestSimEv){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  bestSimEvScores <- rbind(bestSimEvScores,df)
  rm(df)
}
bestSimEvScores$alg = as.factor("EvB")

midSimEvScores = data.frame()
for (i in midSimEv){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  midSimEvScores <- rbind(midSimEvScores,df)
  rm(df)
}
midSimEvScores$alg = as.factor("EvM")

worstSimEvScores = data.frame()
for (i in worstSimEv){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  worstSimEvScores <- rbind(worstSimEvScores,df)
  rm(df)
}
worstSimEvScores$alg = as.factor("EvW")

##

######

bestSimNeScores = data.frame()
for (i in bestSimNe){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  bestSimNeScores <- rbind(bestSimNeScores,df)
  rm(df)
}
bestSimNeScores$alg = as.factor("NeB")

midSimNeScores = data.frame()
for (i in midSimNe){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  midSimNeScores <- rbind(midSimNeScores,df)
  rm(df)
}
midSimNeScores$alg = as.factor("NeM")

worstSimNeScores = data.frame()
for (i in worstSimNe){
  filename = paste(results_path,i,sep = "")
  df = read.csv(filename)
  worstSimNeScores <- rbind(worstSimNeScores,df)
  rm(df)
}
worstSimNeScores$alg = as.factor("NeW")

##

######

######## Friedman test aggregated all Sim

## Simulation
rm(R)
agg_results = data.frame()

agg_results <- rbind(simMoScores, simLnScores, simHvScores,
                     bestSimAuScores, midSimAuScores, worstSimAuScores, 
                     bestSimEvScores, midSimEvScores, worstSimEvScores,
                     bestSimNeScores, midSimNeScores, worstSimNeScores)

agg_results$alg <- as.factor(agg_results$alg)
agg_results$task <- as.factor(agg_results$task)

R <- AP.SCI(score~alg+task,data=agg_results,method="Ranks",test="Conover")
## Environment configuration

filename <- file.path(plots_path, paste("release-friedman-sim-portfolio-ranks.tex", sep=""))
linewidth = 5.49733
pdf.dim <- c((0.95*linewidth*0.6),(0.85*linewidth*0.6)) # width, height
tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])

level_order=c("\\texttt{TF-WS-}\\textit{B}",
              "\\texttt{TF-WS-}\\textit{M}",
              "\\texttt{TF-WS-}\\textit{W}",
              "\\texttt{EC-WS-}\\textit{B}",
              "\\texttt{EC-WS-}\\textit{M}",
              "\\texttt{EC-WS-}\\textit{W}",
              "\\texttt{TF-HV}",
              "\\texttt{TF-L2}",
              "\\texttt{Mandarina}",
              "\\texttt{NC-WS-}\\textit{B}",
              "\\texttt{NC-WS-}\\textit{M}",
              "\\texttt{NC-WS-}\\textit{W}")
ggplot(R, aes(x=y,y=level_order)) +
  #scale_x_continuous(breaks=c(0,10,15,20,25,30)) +
  labs(title="{\\footnotesize A. Performance values test}", x="Average rank", y="") +
  geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
  geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
  geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
  geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
  theme_bw() +
  theme(
    plot.title = element_text(hjust = 0.5),
    axis.ticks.length=unit(.25, "cm"),
    panel.grid.major.y =  element_blank(),
    panel.grid.major.x = element_blank(),
    panel.grid.minor = element_blank(),
    panel.border = element_rect(fill=NA, colour="black",size=1),
    axis.text=element_text(colour="black"),
    axis.text.y = element_text(hjust=0, margin = margin(r = 3, unit = "pt")),
    axis.title.x=element_text(colour="black", margin = margin(t = 10))) +
    scale_y_discrete(limits=c("\\texttt{Mandarina}",
                            "\\texttt{TF-L2}",
                            "\\texttt{TF-HV}",
                            "\\texttt{TF-WS-}\\textit{B}",
                            "\\texttt{TF-WS-}\\textit{M}",
                            "\\texttt{TF-WS-}\\textit{W}",
                            "\\texttt{EC-WS-}\\textit{B}",
                            "\\texttt{EC-WS-}\\textit{M}",
                            "\\texttt{EC-WS-}\\textit{W}",
                            "\\texttt{NC-WS-}\\textit{B}",
                            "\\texttt{NC-WS-}\\textit{M}",
                            "\\texttt{NC-WS-}\\textit{W}"))
dev.off()

######## Friedman test aggregated best

## Simulation
rm(R)
agg_results = data.frame()

agg_results <- rbind(simMoScores, simLnScores, simHvScores,
                     bestSimAuScores,  
                     bestSimEvScores, 
                     bestSimNeScores)

agg_results$alg <- as.factor(agg_results$alg)
agg_results$task <- as.factor(agg_results$task)

R <- AP.SCI(score~alg+task,data=agg_results,method="Ranks",test="Conover")
## Environment configuration

filename <- file.path(plots_path, paste("friedman-sim-portfolio-best.tex", sep=""))
linewidth = 5.49733
pdf.dim <- c((0.95*linewidth),(0.7*linewidth)) # width, height
tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])

level_order=c("\\texttt{TuttiFrutti}\\textsubscript{\\textit{Best}}",
              "\\texttt{EvoColor}\\textsubscript{\\textit{Best}}",
              "\\texttt{TuttiFrutti-HV}",
              "\\texttt{Mandarina}",
              "\\texttt{NEATColor}\\textsubscript{\\textit{Best}}")
ggplot(R, aes(x=y,y=level_order)) +
  #scale_x_continuous(breaks=c(0,10,15,20,25,30)) +
  labs(x="Average rank", y="") +
  geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
  geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
  geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
  geom_point(shape = 21, colour = "black", fill = "black", size = 2.0, stroke = 1.0) +
  theme_bw() +
  theme(
    axis.ticks.length=unit(.25, "cm"),
    panel.grid.major.y =  element_blank(),
    panel.grid.major.x = element_blank(),
    panel.grid.minor = element_blank(),
    panel.border = element_rect(fill=NA, colour="black",size=1),
    axis.text=element_text(colour="black"),
    axis.title.x=element_text(colour="black", margin = margin(t = 10))) +
  scale_y_discrete(limits=c("\\texttt{Mandarina}",
                            "\\texttt{TuttiFrutti-HV}",
                            "\\texttt{TuttiFrutti}\\textsubscript{\\textit{Best}}",
                            "\\texttt{EvoColor}\\textsubscript{\\textit{Best}}",
                            "\\texttt{NEATColor}\\textsubscript{\\textit{Best}}"))
dev.off()

## Save portfolios #####################################################################

# saveRDS(bestPortfolio, "bestPortfolioAll.rds")
# saveRDS(midPortfolio, "midPortfolioAll.rds")
# saveRDS(worstPortfolio, "worstPortfolioAll.rds")
# For loading:
# bestPortfolio <- readRDS("bestPortfolio.rds")
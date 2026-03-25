##################################################
# Scatter plots for factory experiments
##################################################
# David Garzon-Ramos (dagarzonramos(at)gmail.com)
##################################################


library(scales) # to access break formatting functions
library(patchwork)

## Rstudio must be opened from the directory of the script

## Cleaning environment

rm(list=ls())

## Set path of files

results_path = "./data/"
plots_path = "./plots/best-conf/"

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

######

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

##

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

######

######## Friedman test to select best mandarina instances

plots_path = "./plots/best-conf/mo/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = simMoScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
    
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}

######## Friedman test to select best HV instances

plots_path = "./plots/best-conf/hv/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = simHvScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
      
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}

######## Friedman test to select best L2Norm instances

plots_path = "./plots/best-conf/ln/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = simLnScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
      
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}

######## Friedman test to select best TuttiFrutti instances

plots_path = "./plots/best-conf/au/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = bestSimAuScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
      
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}

######## Friedman test to select best EvoColor instances

plots_path = "./plots/best-conf/ev/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = bestSimEvScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
      
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}

######## Friedman test to select best NEAT instances

plots_path = "./plots/best-conf/ne/"
rm(R)
rm(best_ctrl)
rm(sub_ctrl)
best_ctrl = bestSimNeScores

# Modify the factor level
new_levels <- c("INS0", "INS1", "INS2", "INS3", "INS4", "INS5", "INS6", "INS7", "INS8", "INS9")  # The new value you want to assign
best_ctrl$alg <- factor(best_ctrl$alg, levels = c(levels(best_ctrl$alg), new_levels))

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 1100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 1101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 1102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 1103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 1104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 1105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 1106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 1107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 1108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 1109] <- "INS9"

# Assign new levels
best_ctrl$alg[best_ctrl$seed == 2100] <- "INS0"
best_ctrl$alg[best_ctrl$seed == 2101] <- "INS1"
best_ctrl$alg[best_ctrl$seed == 2102] <- "INS2"
best_ctrl$alg[best_ctrl$seed == 2103] <- "INS3"
best_ctrl$alg[best_ctrl$seed == 2104] <- "INS4"
best_ctrl$alg[best_ctrl$seed == 2105] <- "INS5"
best_ctrl$alg[best_ctrl$seed == 2106] <- "INS6"
best_ctrl$alg[best_ctrl$seed == 2107] <- "INS7"
best_ctrl$alg[best_ctrl$seed == 2108] <- "INS8"
best_ctrl$alg[best_ctrl$seed == 2109] <- "INS9"

######## Friedman test aggregated all Sim and Psr oracles

## Generate plots
plot_number = 0
for (k in seq(0,5)){
  for (l in seq(0,5)){
    if (k<l){
      
      plot_name <- paste("p", k,l, sep = "")
      
      filename <- file.path(plots_path, paste("best-", plot_name, ".tex", sep=""))
      linewidth = 5.49733
      pdf.dim <- c((0.265*linewidth*1.2),(0.265*linewidth*1.325)) # width, heigh
      tikz(file = filename, width=pdf.dim[1], height=pdf.dim[2])
      
      task_b = paste("t",k,l,"B", sep = "")
      task_r = paste("t",k,l,"R", sep = "")
      
      tasks_to_keep <- c(task_b, task_r)
      
      sub_ctrl <- best_ctrl[best_ctrl$task %in% tasks_to_keep, ]
      
      sub_ctrl$alg <- as.factor(sub_ctrl$alg)
      sub_ctrl$task <- as.factor(sub_ctrl$task)
      
      R <- AP.SCI(score~alg+task,data=sub_ctrl,method="Ranks",test="Conover")
      
      level_order=new_levels
      
      plot_number = plot_number + 1
      plot_title = paste("{\\tiny",plot_number,". $m_{\\texttt{S}",k+1,".\\texttt{S}",l+1,"}$}", sep = "")
      
      p <- ggplot(R, aes(x=y,y=level_order)) +
        labs(x="Average rank", y="") +
        #geom_vline(aes(xintercept=upper),  color="light gray", size=0.2) +
        #geom_vline(aes(xintercept=lower), color="light gray", size=0.2) +
        geom_vline(aes(xintercept=y), color="light gray", size=0.2) +
        geom_errorbarh(aes(xmax=upper, xmin=lower,height=0.5)) +
        geom_point(shape = 21, colour = "black", fill = "black", size = 0.84, stroke = 0.6) +
        theme_bw() +
        ggtitle(plot_title) +
        theme(
          axis.ticks.length=unit(.25, "cm"),
          panel.grid.major.y =  element_blank(),
          panel.grid.major.x = element_blank(),
          panel.grid.minor = element_blank(),
          panel.border = element_rect(fill=NA, colour="black",size=1),
          axis.text=element_text(colour="black"),
          axis.title.x=element_text(colour="black", margin = margin(t = 2)))
      
      plot(p)
      #show(p)
      #Sys.sleep(2)
      dev.off()
      
    }
  }
}
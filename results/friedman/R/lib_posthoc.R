#require(combinat)
#require(Hmisc)
#require(lattice)

example.ci <- function()
  {
    S <- cbind(expand.grid(instance=c("inst1","inst2"),algo=c("alg1","alg2"),class=c("boh1"),n=c(1,2,3,4)),res=runif(16,min=1,max=3))
    print(S)
    
    R <- AP.SCI(res~algo*instance,data=S,method="Parametric",test="Tukey")
    plot.ci(R)

    R <- AP.SCI(res~algo+instance,data=S,method="Ranks",test="Conover")
    plot.ci(R)
    
	
	
    S <- cbind(expand.grid(instance=c("inst1","inst2"),algo=c("alg1","alg2"),class=c("boh1","boh2"),n=c(1,2)),res=runif(16,min=1,max=3))

    R <- AP.SCI.wrapper.class(res~algo*instance,data=S,method="Parametric",test="Tukey")
    plot.ci.xclass(R)
	intervals(algo~y|class,R,scales=list(relation="free"),par.strip.text=list(cex=0.6),cex=0.7)
	
   	R <- AP.SCI.wrapper.class(res~algo+instance,data=S,method="Ranks",test="Conover")
   	intervals(algo~y|class,R,scales=list(relation="free"),par.strip.text=list(cex=0.6),cex=0.7)
	
  }


AP.SCI.wrapper.class <- function(formula,data=NULL,method=NULL,test=NULL,adj.method=NULL,B=NULL,alpha=0.05,type="csp",...)
  {
    out <- data.frame()
    for (c in levels(data$class))
      {
        cat(c," ")
        X1 <- data[data$class==c,]
        if (dim(X1)[1] != 0)
          {
            if (method=="Parametric")
              {
                R <-AP.SCI(formula,data=X1,method,test,alpha)
              }
            if (method=="Ranks")
              {
                R <-AP.SCI(formula,data=X1,method,test,alpha)
              }
            if (method=="Permutations")
              {
                R <-AP.SCI(formula,data=X1,method,test,adj.method,B,alpha,type)
                ##print(R)
              }
            cl <- paste(c,"   (",length(unique(X1[[ attr(terms(formula),"term.labels")[2]]]))," Instances)",sep="")
            ##R <- R[order(R$y),]
            out <- rbind(out,data.frame(R,class=cl,test=test))
          }
      }
    return(out)
  }

AP.SCI <- function(formula,data=NULL,method=NULL,test=NULL,adj.method=NULL,B=NULL,alpha=0.05,type="csp",...)
  {
   
    methods <- c("Parametric","Permutations","Ranks")
    if (missing(method) || !(method %in% methods)) 
      stop(paste("'method'", deparse(methods),"must be supplied as a name"))

    csp.usp <- ifelse(type=="usp",2,1)
    
    Terms <- terms(formula)

    T <- model.frame(formula,data,drop.unused.levels=TRUE)
    eff.tested <- attr(Terms,"term.labels")[1]
    eff.block <- attr(Terms,"term.labels")[2] 
    response <- deparse(Terms[[2]], width = 500, backtick = TRUE)
    
    T<-T[order(T[[eff.block]],T[[eff.tested]]),]
    runss <- table(T[[eff.tested]],T[[eff.block]])
    if (length(unique(runss)) > 1)
      stop("Experiment unbalanced: experimental units have not the same number of observations\nThis method does not handle this case\n")
    
    repeated <- FALSE
    interaction <- FALSE
    runs <- unique(runss)
    if (runs>1)
      {
        repeated <- TRUE
        if (Terms[[3]][[1]]== "*")
          interaction = TRUE
      }
   
    A <- array(
               T[[response]],
               dim=c(runs,nlevels(T[[eff.tested]]),nlevels(T[[eff.block]])),
               dimnames=list(NULL,levels(T[[eff.tested]]),levels(T[[eff.block]]))
               )
    
    if (method=="Parametric")
      {
        tests <- c("Tukey","LSD","LSDBonferroni","Sheffe")
        if (missing(test) || !(test %in% tests)) 
          stop(paste("'test'", deparse(tests), "must be supplied as a name"))
        
        if (test=="Tukey")
          {
            ecall <- match.call(aov,call("aov",
                                         formula=as.formula(formula), #environment(formula)),
                                         data=data
                                         )
                                )
            l <- eval(ecall,parent.frame())
            ##
            tk<- TukeyHSD(l,eff.tested,
                         conf.level=1-alpha,ordered=TRUE)
				 
            MSD <- max(abs(tk[[eff.tested]][,3]-tk[[eff.tested]][,2]))/2
            A.i <- apply(A,2,mean)
            algs <- as.character(unlist(dimnames(A)[2]))
            OUT <- data.frame(algo=factor(algs,levels=algs),y=as.numeric(A.i),
                              lower=as.numeric((A.i-MSD/2)),upper=as.numeric((A.i+MSD/2)))
          }
        else
          {
            OUT <- MSD.parametric(A,test,interaction,alpha)
          }
      }
    if (method=="Permutations")
      {
        if (missing(B))
          stop("'B' number of permutations must be supplied as integer")
        if (missing(adj.method))
          stop("adj.method not given")

        if (adj.method=="simul.Bonferroni")
          OUT <- MSD.permutation.Pesarin.C(A,B,interaction,alpha,1,csp.usp)
        else if (adj.method=="none")
          OUT <- MSD.permutation.Pesarin.C(A,B,interaction,alpha,0,csp.usp)

        else if (adj.method=="Bonferroni")
          OUT <- MSD.different.C(A,B,interaction,alpha,csp.usp)
      }
    if (method=="Ranks")
      {
        tests <- c("Conover","Sheskin","Permutations")
        if (missing(test) || !(test %in% tests)) 
          stop("'test'", deparse(tests) ,"must be supplied as a name")
        OUT <- MSD.ranks.Friedman(A,test,alpha)
      }
    ##OUT <- OUT[order(OUT$y),]
    return(OUT)
    
  }



MSD.parametric <- function(A,testname=NULL,interaction=TRUE,alpha.fw=0.05)
  {
 
    r <- dim(A)[1] #number of runs
    k <- dim(A)[2] #number of treatments
    b <- dim(A)[3] #number of blocks
    
    N <- b*r
   
#############################################
    c <- k*(k-1)/2
    alpha.pc <- alpha.fw/c
#############################################
    
    if (r>1)
      {
        A.i. <- apply(A,2,mean)
        Ah.. <- apply(A,3,mean)
        A... <- mean(A)
        Ahi. <- apply(A,c(2,3),mean)
        if (interaction)
          {##with interaction
            MSE <- sum(
                       (A - array(rep(Ahi.,each=3),dim=dim(A))
                        )^2
                       )/(b*k*(r-1)) ##testato e' ok
            if (testname=="LSD")
              {
                MSD <- qt(alpha.fw/2,b*k*r-b*r,lower.tail=FALSE) * sqrt(2*MSE/(b*r))
              }
            if (testname=="LSDBonferroni")
              {
                MSD <- qt(alpha.pc/2,b*k*r-b*r,lower.tail=FALSE)*sqrt(2*MSE/(b*r))
              }
            if (testname=="Sheffe")
              {
                MSD <- sqrt((k-1)*qf(alpha.fw,k-1,b*k*r-b*r,lower.tail=FALSE)) * sqrt(2*MSE/(b*r))
              }
          }
        else
          {  ##without interaction
             MSE <- sum(
                 (A
                  - array(rep(rep(A.i.,each=r),b),dim=dim(A))
                  - array(rep(Ah..,each=k*r),dim=dim(A))
                  + A...
                  )^2
                 )/(b*k*r-b-k+1) ##testato e' ok
             
            if (testname=="LSD")
              {
                MSD <- qt(alpha.fw/2,b*k*r-b-k+1,lower.tail=FALSE) * sqrt(2*MSE/(b*r))
              }
            if (testname=="LSDBonferroni")
              {
                MSD <- qt(alpha.pc/2,b*k*r-b-k+1,lower.tail=FALSE)*sqrt(2*MSE/(b*r))
              }
            if (testname=="Sheffe")
              {
                MSD <- sqrt((k-1)*qf(alpha.fw,k-1,b*k*r-b-k+1,lower.tail=FALSE)) * sqrt(2*MSE/(b*r))
              }
          }
        
        algs <- as.character(unlist(dimnames(A)[2]))
        O <- data.frame(algo=factor(algs,levels=algs),y=as.numeric(A.i.),
                        lower=as.numeric((A.i.-MSD/2)),upper=as.numeric((A.i.+MSD/2)))
      }
    else
      {
        M <- A[1,,]
        M.i <- apply(M,1,mean)
        Mh. <- apply(M,2,mean)
        M.. <- mean(M)

        MSE <- sum(
                   (M
                    - array(M.i,dim=dim(M))
                    - array(rep(Mh.,each=k),dim=dim(M))
                    + M..
                    )^2
                   )/(b*k-b-k+1) ##testato e' ok

        if (testname=="LSD")
          {
            MSD <- qt(alpha.fw/2,b*k-b-k+1,lower.tail=FALSE) * sqrt(2*MSE/(b))
          }
        if (testname=="LSDBonferroni")
          {
            MSD <- qt(alpha.pc/2,b*k-b-k+1,lower.tail=FALSE)*sqrt(2*MSE/(b))
          }
        if (testname=="Sheffe")
          {
            MSD <- sqrt((k-1)*qf(alpha.fw,k-1,b*k-b-k+1,lower.tail=FALSE)) * sqrt(2*MSE/(b))
          }

        algs <- as.character(unlist(dimnames(A)[2]))
        O <- data.frame(algo=factor(algs,levels=algs),y=as.numeric(M.i),
                        lower=as.numeric((M.i-MSD/2)),upper=as.numeric((M.i+MSD/2)))
      }
    return(O)
  }


########################################################################################
MSD.ranks.Friedman <- function(A,testname=NULL,alpha.fw=0.05,B=1000,type="csp")
  {
    #source("/u/kika/staff/machud/Progs/statistics/lib_effects.R")
    r <- dim(A)[1] 
    k <- dim(A)[2] #number of treatments
    b <- dim(A)[3] #number of blocks
    
    #print("Vars¨")
    #show(r)
    #show(k)
    #show(b)
    #show(A)
    
    N <- b*r

    if (r>1)
      {
        cat("Two Ways with replicates Experiment: Perform Friedman Test\n")
        
        R <- array(dim=c(r,k,b),dimnames=dimnames(A))
        
        #show(R)

        for (h in 1:b)
          {
            R[,,h] <- matrix(rank(A[,,h]),nrow=k,byrow=FALSE)
          }

        R.i. <- apply(R,2,sum)

        if (testname=="Conover")
          {
            ER.i. <- (b*r*(r*k+1))/2
            VarR.i. <- (r*(k-1))/(k*(r*k-1)) * (sum(R^2) - (r*k*b*(r*k+1)^2)/4)
            df <- k*r*b - k - b + 1 
            
            T <- sum((k-1)/k * ((R.i. - ER.i.)^2)/VarR.i.)

            p <- pchisq(T,k-1) ##perform Friedman test

            ##Check 
            if (p <= (1-alpha.fw))
              {
                algs <- as.character(unlist(dimnames(A)[2]))
                O <- data.frame(algo=factor(algs,levels=algs),
                                y=as.numeric(R.i./N),lower=as.numeric((R.i./N-(k*r)/2)),
                                upper=as.numeric((R.i./N+(k*r)/2)))
                cat("Friedman test, main effects: No differences in the levels\n") 
                return(O)
              }
            t.value <- qt((alpha.fw/2),df,lower.tail=FALSE)
            MSD <- t.value * sqrt(
                                  (2*k*(r*k-1)*VarR.i.)/(b*r^2*(k-1)*df) *
                                  (1-T/(b*(r*k-1)))
                                  )
          }
        if (testname=="Sheskin")##wrong!
          {
            cat("This test is wrong!!\n")
            p.pc <- alpha.fw/(k*(k-1)) #Bonferroni
            t.value <- qnorm(p.pc,0,1,lower.tail=FALSE)
            MSD <- t.value * sqrt(
                                  ((k*r+1)*(b*r+1))/(6*N)
                                  ) #<- Sheskin
          }
        if (testname=="Permutations")
          {
            return(MSD.permutation.Pesarin.C(R,B=B,interaction=TRUE,alpha.fw=0.05,adj=0,csp.usp=1))
          }

        algs <- as.character(unlist(dimnames(A)[2]))
        O <- data.frame(algo=factor(algs,levels=algs),
                        y=as.numeric(R.i./N),lower=as.numeric((R.i./N-MSD/2)),
                        upper=as.numeric((R.i./N+MSD/2)))
      }
    else
      {
        cat("Two Ways Single Measure Experiment: Perform Friedman Test\n")
        M <- A[1,,]
        R <- apply(M,2,rank)
        R.i <- apply(R,1,sum)

        if (testname=="Conover")
          {
            df <- (k-1)*(b-1)
            A1 <- sum(R^2)
            C <- b*k*(k+1)^2/4
            T1 <- (k-1)*sum((R.i-b*(k+1)/2)^2)/(A1-C)
            T2 <- (b-1)*T1/(b*(k-1)-T1)
            
            p <- pf(T2,k-1,df) ##perform Friedman test
            
            if (p <= (1-alpha.fw))
              {
                algs <- as.character(unlist(dimnames(A)[2]))
                O <- data.frame(algo=factor(algs,levels=algs),
                                y=as.numeric(R.i/N),lower=as.numeric((R.i/N-k/2)),
                                upper=as.numeric((R.i/N+k/2)))
                #cat("No differences in the levels\n") 
                return(O)
              }
            t.value <- qt((alpha.fw/2),df,lower.tail=FALSE)
            MSD <- t.value * sqrt(
                                  (2*(b*A1 - sum(R.i^2)))/
                                  (b^2*(b-1)*(k-1))
                                  )
          }
        if (testname=="Sheskin")##wrong!
          {
            cat("This test is wrong!!\n")
            p.pc <- alpha.fw/(k*(k-1)) #Bonferroni
            t.value <- qnorm(p.pc,0,1,lower.tail=FALSE)
            MSD <- t.value * sqrt(
                                  (k*(k+1))/(6*b)
                                  ) #<- Sheskin
          }


        algs <- as.character(unlist(dimnames(A)[2]))
        O <- data.frame(algo=factor(algs,levels=algs),
                        y=as.numeric(R.i/N),lower=as.numeric((R.i/N-MSD/2)),
                        upper=as.numeric((R.i/N+MSD/2)))
      }
    return(O)
  }






#############################################################################################################
MSD.permutation.Pesarin.C <- function(A,B=NULL,interaction=TRUE,alpha.fw=0.05,adj=0,csp.usp=1)#%interaction=TRUE)
{
  #require("combinat")
  #dyn.load(paste("/home/marco/Library/R/permutationR",.Platform$dynlib.ext,sep=""))
  ##  if (missing(interaction))
  ##    {
  ##      interaction <- FALSE
  ##      cat("model without interaction\n")
  ##    }
  ##  #require(Hmisc)
###cat(paste("compute permutation",B,"MSD..."))
  
  r <- dim(A)[1] #number of runs
  k <- dim(A)[2] #number of treatments
  b <- dim(A)[3] #number of blocks
  N <- b*r

  ##prob <- sapply(0:r, function(x) {nCm(r,x)^(b*2)})
  ##prob <- prob/sum(prob)
  
#############################################################
 
  epsilon <- 1/B
  c <- k*(k-1)/2
  if (adj==1)
    {
      alpha.pc <- alpha.fw/c
      if (alpha.pc < 2/nCm(2*r,r) & csp.usp == 1)
        {
          cat("Attention: I must adapt the per-comparison error level: from ",alpha.pc,"to",2/nCm(2*r,r),"\n")
          alpha.pc <- 2/nCm(2*r,r)
        }
    }
  else
    alpha.pc <- alpha.fw
#############################################################
  
  A.i. <- apply(A,2,mean)
  Ah.. <- apply(A,3,mean)
  A... <- mean(A)
  Ahi. <- apply(A,c(2,3),mean)
  if (interaction)
    {##with interaction
      MSE <- sum(
                 (A
                  - array(rep(Ahi.,each=r),dim=dim(A))
                  )^2
                 )/(b*k*(r-1)) ##testato e' ok
      ni <- qt(alpha.fw/2,b*k*r-b*k,lower.tail=FALSE)*sqrt(2*MSE/(b*r)) ##LSD
      ##tk<-TukeyHSD(aov(err1 ~ alg*inst,data=F),"alg",conf.level=0.95,ordered=TRUE)
      ##ni <- max(abs(tk$"alg"[,3]-tk$"alg"[,2]))/2
    }
  else
    {
      ##without interaction
      MSE <- sum(
                 (A
                  - array(rep(rep(A.i.,each=r),b),dim=dim(A))
                  - array(rep(Ah..,each=k*r),dim=dim(A))
                  + A...
                  )^2
                 )/(b*k*r-b-k+1) ##testato e' ok
      ni <- qt(alpha.fw/2,b*k*r-b-k+1,lower.tail=FALSE)*sqrt(2*MSE/(b*r)) ##LSD
    }

  #print(ni)
  exact <- FALSE
  if (csp.usp==1 & r>1 & r<=7)
    {
      exact <- TRUE
      tmp <- t(combn(2*r,r))
      tmp <- tmp[1:(dim(tmp)[1]/2),]
      permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
    }
  if (csp.usp==2 & r > 1 & b > 1)
    {
      if ((2*b)*(k*(k-1)/2)<100)
        {
          if (r %% 2 == 0)
            {
              n <- r/2+1
              nu.probabilities <- sapply(0:(n-2),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
              nu.probabilities[n] <- (nCm(r,n-1)^(2*b)/2)^(k*(k-1)/2)
              nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
            }
          else
            {
              n <- (r+1)/2
              nu.probabilities <- sapply(0:(n-1),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
              nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
            }
                                        #nu.probabilities <- sapply(0:r,function(x) nCm(r,x)^(2*b))
                                        #nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
                                        #nu.probabilities[r+1] <- 1
        }
      else
        {
          cat("Overflow: Scambio sempre tutti",(2*b)*(k*(k-1)/2),"\n")  
          if (r %% 2 == 0)
            {
              n <- r/2+1
              nu.probabilities <- rep(0,(n-1))
              nu.probabilities[n] <- 1
            }
          else
            {
              n <- (r+1)/2
              nu.probabilities <- rep(0,(n-1))
              nu.probabilities[n] <- 1
            }
          #print(nu.probabilities)
        }

    }
  else
    {
      nu.probabilities <- 1
    }

  if (exact)
    {
      MSD <- .C("APC_permutation_exact", as.double(A), as.double(A.i.), as.integer(k),
                as.integer(b), as.integer(r), as.double(nu.probabilities),
                ni=as.double(ni/10),
                as.double(alpha.pc),
                as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                as.integer(csp.usp))$ni
    }
  else
    {
      MSD <- .C("APC_permutation_CMC", as.double(A), as.double(A.i.), as.integer(k),
                as.integer(b), as.integer(r), as.double(nu.probabilities),
                ni=as.double(ni/10),
                as.double(alpha.pc),  as.integer(B), as.integer(csp.usp))$ni
    }
  #MSD <- ni
  #print(MSD)
  #print(ni)
  
  
  #cat("Final in R",MSD,"\n")
  algs <- as.character(unlist(dimnames(A)[2]))
  O <- data.frame(algo=factor(algs,levels=algs),y=as.numeric(A.i.),
                  lower=as.numeric((A.i.-MSD/2)),upper=as.numeric((A.i.+MSD/2)))
  return(O)
}




MSD.different.C <- function(A,B=NULL,interaction=TRUE,alpha.fw=0.05,csp.usp=1)
  {
    #require(combinat)
    #dyn.load(paste("/home/marco/Library/R/permutationR",.Platform$dynlib.ext,sep=""))

    
    r <- dim(A)[1] #number of runs
    k <- dim(A)[2] #number of treatments
    b <- dim(A)[3] #number of blocks
    N <- b*r

 
#############################################################
 
  epsilon <- 1/B
  c <- k*(k-1)/2
  alpha.pc <- alpha.fw/c
  if (alpha.pc < 2/nCm(2*r,r) & csp.usp == 1)
    {
      cat("Attention: I must adapt the per-comparison error level: from ",alpha.pc,"to",2/nCm(2*r,r),"\n")
      alpha.pc <- 2/nCm(2*r,r)
    }
  
#############################################################
  
    
  A.i. <- apply(A,2,mean)
  Ah.. <- apply(A,3,mean)
  A... <- mean(A)
  Ahi. <- apply(A,c(2,3),mean)
  if (interaction)
    {##with interaction
      MSE <- sum(
                 (A
                  - array(rep(Ahi.,each=r),dim=dim(A))
                  )^2
                 )/(b*k*(r-1)) ##testato e' ok
      ni <- qt(alpha.pc/2,b*k*r-b*k,lower.tail=FALSE)*sqrt(2*MSE/(b*r)) ##LSD
      ##tk<-TukeyHSD(aov(err1 ~ alg*inst,data=F),"alg",conf.level=0.95,ordered=TRUE)
      ##ni <- max(abs(tk$"alg"[,3]-tk$"alg"[,2]))/2
    }
  else
    {
      ##without interaction
      MSE <- sum(
                 (A
                  - array(rep(rep(A.i.,each=r),b),dim=dim(A))
                  - array(rep(Ah..,each=k*r),dim=dim(A))
                  + A...
                  )^2
                 )/(b*k*r-b-k+1) ##testato e' ok
      ni <- qt(alpha.pc/2,b*k*r-b-k+1,lower.tail=FALSE)*sqrt(2*MSE/(b*r)) ##LSD
    }


    exact <- FALSE
    if (csp.usp==1 & r>1 & r<=7)
      {
        exact <- TRUE
        tmp <- t(combn(2*r,r))
        tmp <- tmp[1:(dim(tmp)[1]/2),]
        permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
      }
    
    if (csp.usp==2 & r > 1 & b > 1 & (2*b)*(k*(k-1)/2)<100)
      {
          if (r %% 2 == 0)
        {
          n <- r/2+1
          nu.probabilities <- sapply(0:(n-2),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
          nu.probabilities[n] <- (nCm(r,n-1)^(2*b)/2)^(k*(k-1)/2)
          nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
        }
      else
        {
          n <- (r+1)/2
          nu.probabilities <- sapply(0:(n-1),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
          nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
        }
          #nu.probabilities <- sapply(0:r,function(x) nCm(r,x)^(2*b))
        #nu.probabilities <- cumsum(round(nu.probabilities/sum(nu.probabilities),8))
        #nu.probabilities[r+1] <- 1
      }
    else
      {
        nu.probabilities <- 1
      }
    
    algs <- as.character(unlist(dimnames(A)[2]))
    #print(ni)
    O <- data.frame()
    
    for ( i in 1:(k-1))
      {
        for (j in (i+1):k)
          {
            if (exact)
              {
                MSD <- .C("Pairwise_permutation_exact", as.double(A[,c(i,j),]),
                          as.double(A.i.[i]-A.i.[j]),
                          as.integer(b), as.integer(r), as.double(nu.probabilities),
                          ni=as.double(ni/10), 
                          as.double(alpha.pc),
                          as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                          as.integer(csp.usp))$ni
                                        #cat("Final in R",MSD,"\n")
              }
            else
              {
                 MSD <- .C("Pairwise_permutation_CMC", as.double(A[,c(i,j),]),
                          as.double(A.i.[i]-A.i.[j]),
                          as.integer(b), as.integer(r), as.double(nu.probabilities),
                          ni=as.double(ni/10), 
                          as.double(alpha.pc), as.integer(B), as.integer(csp.usp))$ni
                                        #cat("Final in R",MSD,"\n")
              }
           
            O <- rbind(O,data.frame(algo1=algs[i],algo2=algs[j],
                                    y1=as.numeric(A.i.[i]),
                                    y2=as.numeric(A.i.[j]),
                                    #lower=as.numeric((A.i.[i]-MSD/2)),
                                    #upper=as.numeric((A.i.[j]+MSD/2)),
                                    MSD=MSD))
            
          }
      }
    return(O)
    
  }

 


########################################################################################
########################################################################################
########################################################################################
plot.hsu <- function(O,file,title,type="eps",dev="eps")
  {
    if (type=="pdf")
      {
        pdf(file=paste("hsu-",file,".pdf",sep=""),
            onefile=FALSE,horizontal=FALSE,
            width=5,height=5,family="Helvetica")
      }
    else if (type=="eps")
      {
        postscript(file=paste("hsu-",file,".eps",sep=""),
                   onefile=FALSE,horizontal=FALSE,
                   width=4,height=4,family="Helvetica")
      }
    else if (type=="tex")
      {
        pictex(file=paste("hsu-",file,".tex",sep=""),
               width=4,height=4)#,family="Helvetica")
      }
    else
      {
        stop("set device\n")
      }
                                        #pdf(file="hsu.pdf",onefile=FALSE,horizontal=FALSE,
        #width=5,height=5,
   #     paper="a4",
                                        #     family="Helvetica")

    cex <- 1
    cex.names <- 0.9
    O <- O[order(O$y1,O$y2),]
    par(mar=c(4,4,2,3),
        mgp=c(2,0.5,0),
        tck=-0.01,
        cex=cex,cex.lab=cex,
        font.main=1,
        xlog=TRUE,
        ylog=TRUE)
    pmax <- max(O$y1,O$y2)+max(O$MSD)
    #pmin <- min(O$y1,O$y2)-min(O$MSD)
    pmin <- min(O$y1,O$y2)-min(O$MSD)
    
    plot(c(pmin,pmax),c(pmin,pmax),
         main="",xlab="",ylab="",type="n",axes=FALSE)#,xaxt="n",yaxt="n")
    box()
   
    
    axis(3,cex.axis=cex.names)
    axis(4,las=1,cex.axis=cex.names)
    #abline(v=c(O$y1,O$y2),col="lightgray")
    #abline(h=c(O$y1,O$y2),col="lightgray")

    segments(c(O$y1,O$y2),par("usr")[3],c(O$y1,O$y2),par("usr")[4],col="lightgrey")
    segments(par("usr")[1],c(O$y1,O$y2),par("usr")[2],c(O$y1,O$y2),col="lightgrey")
    
    curve((x),par("usr")[1],par("usr")[2],add=TRUE,lwd=2)
    algs <- c(as.character(O$algo1),as.character(O$algo2))
    means <- c(O$y1,O$y2)
    s <- data.frame(algs,means)
    u <- aggregate(s$means,list(algs=s$algs),unique)
    u <- u[order(u$x),]
    
    axis(1,at=u$x,labels=FALSE,tick = TRUE,las=2)
    #pr <- pretty(x)
    par(xpd=TRUE)

    at <- u$x
    nalgs <- as.character(u$algs)
    quali <- seq(1,length(at),2)
    nquali <- seq(2,length(at),2)
    
    text(at[nquali], par("usr")[3]-(par("usr")[4]-par("usr")[3])/40, nalgs[nquali], adj=1, srt=45,cex=cex.names) 
    
    axis(2,at=u$x,labels=FALSE,tick = TRUE,las=1)#,cex.axis=cex.names)
    text( par("usr")[1]-(par("usr")[2]-par("usr")[1])/40,at[quali], nalgs[quali], adj=1, srt=0,cex=cex.names) 
   
    
    A <- O[O$y1>=O$y2,]
    if (dim(A)[1]>0)
      segments(A$y1-A$MSD/2,
               A$y2+A$MSD/2,
               A$y1+A$MSD/2,
               A$y2-A$MSD/2,lwd=3,col="grey50")
    B <- O[O$y2>O$y1,]
    if (dim(B)[1]>0)
      segments(B$y2-B$MSD/2,
               B$y1+B$MSD/2,
               B$y2+B$MSD/2,
               B$y1-B$MSD/2,lwd=3,col="grey50")

    A <- A[abs(A$y1-A$y2)<A$MSD,]
    if (dim(A)[1]>0)
      segments(A$y1-A$MSD/2,
               A$y2+A$MSD/2,
               A$y1+A$MSD/2,
               A$y2-A$MSD/2,lwd=3)
    B <- B[abs(B$y1-B$y2)<B$MSD,]
    if (dim(B)[1]>0)
      segments(B$y2-B$MSD/2,
               B$y1+B$MSD/2,
               B$y2+B$MSD/2,
               B$y1-B$MSD/2,lwd=3)
    mtext(side=3,text=title,line=-2)
     dev.off()
  
  }



plot.multi.hsu <- function(h,disp)
{
  postscript(file="hsu.eps",onefile=FALSE,horizontal=FALSE,
             width=5,height=5,family="Helvetica")
                                        #pdf(file="hsu.pdf",onefile=FALSE,horizontal=FALSE,
        #width=5,height=5,
   #     paper="a4",
                                        #     family="Helvetica")

  par( mfrow = disp )
  for ( c in levels(h$class) )
    {
      H <- h[h$class==c,]
      H$inst<-factor(H$inst, levels=unique(H$inst))
      plot.hsu(H)
    }
   dev.off()
  
}


intervals <- function(formula,IN, ...)
  {
    require(lattice)

    
    IN[[deparse(formula[[2]])]] <- factor(IN[[deparse(formula[[2]])]],
                                          levels=levels(IN[[deparse(formula[[2]])]])[order(IN[[deparse(formula[[3]][[2]])]])])
    ## prepanel and panel function for displaying confidence intervals
    
    prepanel.ci <- function(x, y, lx, ux, subscripts, ...)
      {
        x <- as.numeric(x)
        lx <- as.numeric(lx[subscripts])
        ux <- as.numeric(ux[subscripts])
        list(xlim = range(x, ux, lx, finite = TRUE))
      }
    
    
    panel.ci <- function(x, y, lx, ux, subscripts, pch = 16, ...)
      {
        x <- as.numeric(x)
        y <- as.numeric(y)
        lx <- as.numeric(lx[subscripts])
        ux <- as.numeric(ux[subscripts])
        panel.abline(h = unique(y), col = "grey")
        panel.abline(v=c(lx,ux),col="grey85")
        panel.arrows(lx, y, ux, y, ##col = 'black',
                     length = 0.06, ##unit = "native",
                     angle = 90, code = 3, ...)
        panel.xyplot(x, y, pch = pch, ...)
      }
    

         dotplot(formula,data=IN,
                 lx = IN[["lower"]], ux = IN[["upper"]],
                 prepanel = prepanel.ci,
                 panel = panel.ci, ...)
  }



intervals.2D <- function(formula,IN, ...)
  {
    require(lattice)

    ## prepanel and panel function for displaying confidence intervals
    
    prepanel.ci <- function(x, y, lx, ux, ly, uy, names, subscripts, ...)
      {
        x <- as.numeric(x)
        lx <- as.numeric(lx[subscripts])
        ux <- as.numeric(ux[subscripts])
        y <- as.numeric(y)
        ly <- as.numeric(ly[subscripts])
        uy <- as.numeric(uy[subscripts])
        list(xlim = range(x, ux, lx, finite = TRUE), ylim=range(y, uy, ly, finite = TRUE))
      }
    
    
    panel.ci <- function(x, y, lx, ux, ly, uy, names, subscripts, pch = 20, cex.pch=0.2, ... )#c(15:(15+length(names))), col=rainbow(length(names)),...)
      {
        x <- as.numeric(x)
        y <- as.numeric(y)
        lx <- as.numeric(lx[subscripts])
        ux <- as.numeric(ux[subscripts])
        ly <- as.numeric(ly[subscripts])
        uy <- as.numeric(uy[subscripts])
        rx <-  range(y, uy, ly, finite = TRUE)
        ry <-  range(y, uy, ly, finite = TRUE)
        dx <- diff(rx)/70
        dy <- diff(ry)/70
        dx <- 0.04
        dy <- 0.95
        panel.abline(h =c(ly,uy), col = "grey85")
        panel.abline(v=c(lx,ux),col="grey85")
        panel.arrows(lx, y, ux, y,
                     length = 0.0, ##unit = "native",
                     angle = 90, code = 3, ...)
        panel.arrows(x,ly, x,uy, 
                     length = 0.0, ##unit = "native",
                     angle = 90, code = 3, ...)
        panel.xyplot(x, y, pch = pch, ...)
        ltext(x = x+dx, y = y+dy, label = names[subscripts], cex = 0.7, fontfamily = "Helvetica",#"HersheySans", 
              fontface = "italic",adj = c(0,0))
      }
    
    

         xyplot(formula,data=IN,
                names=IN$algo,
                lx = IN$x.lower, ux = IN$x.upper,
                 ly = IN$y.lower, uy = IN$y.upper,
                 prepanel = prepanel.ci,
                 panel = panel.ci, ...)
  }

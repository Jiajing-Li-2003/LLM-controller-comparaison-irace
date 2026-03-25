##require(combinat)
##require(Hmisc)

main.effects.parametric <- function(F)
{
  p <- anova(lm(err1 ~ alg*inst,data=F))[["Pr(>F)"]][1]
  #return(p)
  alpha.fw <- 0.05
  if (p >= alpha.fw)
    return(TRUE)
  else
    return(FALSE)
}


interaction.parametric <- function(F)
{
  p <- anova(lm(err1 ~ alg*inst,data=F))[["Pr(>F)"]][3]
  alpha.fw <- 0.05
  if (p >= alpha.fw)
    return(TRUE)
  else
    return(FALSE)
}


Friedman.test <- function(formula,data=NULL)
{
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
  
  runs <- unique(runss)
  if (runs>1)
    {
      repeated <- TRUE
    }

  A <- array(
             T[[response]],
             dim=c(runs,nlevels(T[[eff.tested]]),nlevels(T[[eff.block]])),
             dimnames=list(NULL,levels(T[[eff.tested]]),levels(T[[eff.block]]))
                                        #dim=c(nlevels(T[[eff.tested]]),nlevels(T[[eff.block]]),runs),
                                        #dimnames=list(levels(T[[eff.tested]]),levels(T[[eff.block]]),NULL)
             )
  
  r <- dim(A)[1] 
  k <- dim(A)[2] #number of treatments
  b <- dim(A)[3] #number of blocks

  N <- b*r
  
  if (r>1)
    {
      #cat("Rank repeated\n")
      
      R <- array(dim=c(r,k,b),dimnames=dimnames(A))
      for (h in 1:b)
        {
          R[,,h] <- matrix(rank(A[,,h]),nrow=k,byrow=FALSE)
        }
      
      R.i. <- apply(R,2,sum)
      
     
          ER.i. <- (b*r*(r*k+1))/2
          VarR.i. <- (r*(k-1))/(k*(r*k-1)) * (sum(R^2) - (r*k*b*(r*k+1)^2)/4)
          df <- k*r*b - k - b + 1 
          
          T <- sum((k-1)/k * ((R.i. - ER.i.)^2)/VarR.i.)
          
          p <- 1-pchisq(T,k-1) ##perform Friedman test
       
    }
  else
    {
      #cat("Rank one measure\n")
      M <- A[1,,]
      R <- apply(M,2,rank)
      R.i <- apply(R,1,sum)
      
     
          df <- (k-1)*(b-1)
          A1 <- sum(R^2)
          C <- b*k*(k+1)^2/4
          T1 <- (k-1)*sum((R.i-b*(k+1)/2)^2)/(A1-C)
          T2 <- (b-1)*T1/(b*(k-1)-T1)
          
          p <- 1-pf(T2,k-1,df) ##perform Friedman test
        
    }
  return(p)
}


permut.aov <- function(formula,data=NULL,method=NULL,B=NULL,type="csp",...)
{
  ##dyn.load(paste("permutationR",.Platform$dynlib.ext,sep=""))
  methods <- c("same","free")
  if (missing(method) || !(method %in% methods)) 
    stop(paste("'method'", deparse(methods),"must be supplied as a name"))

  if (missing(B))
    stop("'B' number of permutations must be supplied as integer")

    csp.usp <- ifelse(type=="usp",2,1)

  result <- list()
  
  #cat(B)
  
  Terms <- terms(formula)
  eff.tested <- attr(Terms,"term.labels")[1]
  eff.block <- attr(Terms,"term.labels")[2]
  formula2 <- as.formula(paste(deparse(formula[[2]], width = 500, backtick = TRUE),
                               "~", eff.block,formula[[3]][[1]],eff.tested))
  
 
  T <- model.frame(formula,data,drop.unused.levels=TRUE)
  response <- deparse(Terms[[2]], width = 500, backtick = TRUE)

  

  ##require(combinat)
  ##F #data.startcutre
  ##B # number of repetitions 
  ##require(Hmisc)
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
    
#  cat(paste("Interaction",ifelse(interaction,"","NOT"),"included in the model\n"))

   A <- array(
               T[[response]],
               dim=c(runs,nlevels(T[[eff.tested]]),nlevels(T[[eff.block]])),
               dimnames=list(NULL,levels(T[[eff.tested]]),levels(T[[eff.block]]))
               #dim=c(nlevels(T[[eff.tested]]),nlevels(T[[eff.block]]),runs),
               #dimnames=list(levels(T[[eff.tested]]),levels(T[[eff.block]]),NULL)
               )
   r <- dim(A)[1] #number of runs
    k <- dim(A)[2] #number of treatments
    b <- dim(A)[3] #number of blocks


  exact <- FALSE
  if (csp.usp==1 & r>1 & r<=7)
    {
      exact <- TRUE
      tmp <- t(combn(2*r,r))
      tmp <- tmp[1:(dim(tmp)[1]/2),]
      permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
    }
 
  
  if (csp.usp == 2 & r > 1 & b > 1 & (2*b)*(k*(k-1)/2)<100)
    {
      if (r %% 2 == 0)
        {
          n <- r/2+1
          nu.probabilities <- sapply(0:(n-2),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
          nu.probabilities[n] <- ((nCm(r,n-1)^(2*b))/2)^(k*(k-1)/2)
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
  
  if (method=="same")
    {
      if (exact) #r>1 & r<=7 & csp.usp==1)
        {
          #tmp <- t(combn(2*r,r))
          #permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
          result[eff.tested] <- .C("MAIN_EFFECTS_permutation_exact", as.double(A),
                             as.integer(k), as.integer(b), as.integer(r),
                             as.double(nu.probabilities),
                             as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                             p = double(1),
                             as.integer(csp.usp))$p
        }
      else 
        {
          result[eff.tested] <- .C("MAIN_EFFECTS_permutation_CMC", as.double(A),
                             as.integer(k), as.integer(b), as.integer(r),
                             as.double(nu.probabilities),
                             as.integer(B),  p = double(1),
                             as.integer(csp.usp))$p
        }
    }
##  if (method=="free")
##    {
##      result[eff.tested] <- .C("MAIN_EFFECTS_permutation_USP_free", as.double(A),
##              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
##              as.integer(B),  p = double(1) )$p
##    }
##
  if (interaction)
    {
      inter <- paste(eff.tested,eff.block,sep="")

      if (exact) #r>1 & r<=7 & csp.usp==1)
        {
          #tmp <- t(combn(2*r,r))
          #permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
          result[inter] <- .C("INTERACTIONS_permutation_exact", as.double(A),
                              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
                              as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                              p = double(1), as.integer(csp.usp) )$p
        }
      else
        {
          result[inter] <- .C("INTERACTIONS_permutation_CMC", as.double(A),
                              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
                              as.integer(B),  p = double(1), as.integer(csp.usp) )$p
          
        }
    }

  Terms <- terms(formula2)
  eff.tested <- attr(Terms,"term.labels")[1]
  eff.block <- attr(Terms,"term.labels")[2]

  T <- model.frame(formula2,data,drop.unused.levels=TRUE)
  response <- deparse(Terms[[2]], width = 500, backtick = TRUE)
  ##require(combinat)
  ##F #data.startcutre
  ##B # number of repetitions 
  ##require(Hmisc)
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
    
    #cat(paste("Interaction",ifelse(interaction,"","NOT"),"included in the model\n"))

   A <- array(
               T[[response]],
               dim=c(runs,nlevels(T[[eff.tested]]),nlevels(T[[eff.block]])),
               dimnames=list(NULL,levels(T[[eff.tested]]),levels(T[[eff.block]]))
               #dim=c(nlevels(T[[eff.tested]]),nlevels(T[[eff.block]]),runs),
               #dimnames=list(levels(T[[eff.tested]]),levels(T[[eff.block]]),NULL)
               )
   r <- dim(A)[1] #number of runs
    k <- dim(A)[2] #number of treatments
    b <- dim(A)[3] #number of blocks

   if (csp.usp==2 & r > 1 & b > 1 & (2*b)*(k*(k-1)/2)<100 )
    {
        if (r %% 2 == 0)
        {
          n <- r/2+1
          nu.probabilities <- sapply(0:(n-2),function(x) nCm(r,x)^((2*b)*(k*(k-1)/2)))
          nu.probabilities[n] <- ((nCm(r,n-1)^(2*b))/2)^(k*(k-1)/2)
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
     # nu.probabilities[r+1] <- 1
    }
  else
    {
      nu.probabilities <- 1
    }
  
   if (method=="same")
    {
      if (exact)
        {
          result[eff.tested] <- .C("MAIN_EFFECTS_permutation_exact", as.double(A),
                                   as.integer(k), as.integer(b), as.integer(r),
                                   as.double(nu.probabilities),
                                   as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                                   p = double(1), as.integer(csp.usp) )$p
        }
      else
        {
           result[eff.tested] <- .C("MAIN_EFFECTS_permutation_CMC", as.double(A),
                              as.integer(k), as.integer(b), as.integer(r),
                                    as.double(nu.probabilities), 
                              as.integer(B),  p = double(1), as.integer(csp.usp) )$p
         }
    }

 if (interaction)
    {
      inter <- paste(eff.tested,eff.block,sep="")

      if (exact) #r>1 & r<=7 & csp.usp==1)
        {
          #tmp <- t(combn(2*r,r))
          #permutazioni <- cbind(tmp,t(sapply(1:dim(tmp)[1],function(x) which(c(1:(2*r)) %nin% tmp[x,]))))
          result[inter] <- .C("INTERACTIONS_permutation_exact", as.double(A),
                              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
                              as.integer(dim(permutazioni)[1]), as.integer(array(unlist(t(permutazioni)))),
                              p = double(1), as.integer(csp.usp) )$p
        }
      else
        {
          result[inter] <- .C("INTERACTIONS_permutation_CMC", as.double(A),
                              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
                              as.integer(B),  p = double(1), as.integer(csp.usp) )$p
          
        }
    }
  
##  if (method=="free")
##    {
##      result[eff.tested] <- .C("MAIN_EFFECTS_permutation_USP_free", as.double(A),
##              as.integer(k), as.integer(b), as.integer(r), as.double(nu.probabilities),
##              as.integer(B),  p = double(1), as.integer(csp.usp) )$p
##    }
## 
   
  result
##  alpha.fw <- 0.05
##  if (p >= alpha.fw)
##    return(TRUE)
##  else
##    return(FALSE)
##  ##return(aT.)
}



#!/usr/bin/env Rscript
# extract_iteration_results.R
#
# Lit un fichier irace .Rdata (objet `iraceResults`) et ecrit un TSV avec une
# ligne par configuration testee, son cout moyen et le nombre d'evaluations sur
# lequel cette moyenne est calculee.
#
# La colonne NEval est importante : dans un round d'une seule iteration,
# certaines configurations n'ont ete evaluees que quelques fois, et leur moyenne
# est donc tres bruitee. C'est ce qui interdit de comparer directement le
# meilleur MeanCost d'un round LLM a l'elite d'irace, moyennee sur beaucoup plus
# d'evaluations.
#
# Usage :
#   Rscript extract_iteration_results.R <irace.Rdata> <sortie.tsv>

args <- commandArgs(trailingOnly = TRUE)
if (length(args) < 2) {
    stop("Usage: Rscript extract_iteration_results.R <irace.Rdata> <output.tsv>")
}

rdata_file  <- args[1]
output_file <- args[2]

if (!file.exists(rdata_file)) {
    stop(paste0("Fichier introuvable : ", rdata_file))
}

load(rdata_file)

if (!exists("iraceResults")) {
    stop("L'objet 'iraceResults' n'a pas ete trouve dans ce fichier .Rdata.")
}

configs    <- iraceResults$allConfigurations
exp_matrix <- iraceResults$experiments

if (is.null(configs) || is.null(exp_matrix)) {
    stop("iraceResults$allConfigurations ou iraceResults$experiments est manquant.")
}

mean_costs <- colMeans(exp_matrix, na.rm = TRUE)
n_evals    <- colSums(!is.na(exp_matrix))

cost_df <- data.frame(
    .ID.     = as.integer(names(mean_costs)),
    MeanCost = as.numeric(mean_costs),
    NEval    = as.integer(n_evals[names(mean_costs)])
)

merged <- merge(configs, cost_df, by = ".ID.")
merged <- merged[!is.na(merged$MeanCost) & !is.nan(merged$MeanCost), ]
merged <- merged[order(merged$MeanCost), ]

if (nrow(merged) == 0) {
    stop("Aucune configuration avec un cout exploitable dans ce fichier.")
}

write.table(merged, file = output_file, sep = "\t", row.names = FALSE,
            quote = FALSE, na = "NA")

cat(sprintf("%d configurations extraites -> %s\n", nrow(merged), output_file))
cat(sprintf("   meilleur : %.2f | pire : %.2f | evaluations par config : %d a %d\n",
            min(merged$MeanCost), max(merged$MeanCost),
            min(merged$NEval), max(merged$NEval)))

# Copyright (C) 2009 - 2012 Dirk Eddelbuettel and Romain Francois
#
# This file is part of Rcpp.
#
# Rcpp is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Rcpp is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Rcpp.  If not, see <http://www.gnu.org/licenses/>.

.rcpp_error_recorder <- function(e){  
    invisible( .Call( "rcpp_error_recorder", e, PACKAGE="mpMapInteractive") )
}

.warningsEnv <- new.env()
.warningsEnv$warnings <- character()

.rcpp_warning_recorder <- function(w){
    .warningsEnv$warnings <- append(.warningsEnv$warnings, w$message)
    invokeRestart("muffleWarning")
}

.rcpp_collect_warnings <- function() {
    warnings <- .warningsEnv$warnings
    .warningsEnv$warnings <- character()
    warnings
}



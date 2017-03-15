;; set variables specific to the project
;; https://www.emacswiki.org/emacs/DirectoryVariables

;; to stop the annoying warnings about safety, add this
;; to your init file:
;; (put 'c4stl-dir 'safe-local-variable #'stringp)
;; (put 'company-clang-arguments 'safe-local-variable #'listp)

((nil . ((eval . (set (make-local-variable 'c4stl-dir)
                      (file-name-directory
                       (let ((d (dir-locals-find-file ".")))
                         (if (stringp d) d (car d))))))
         (eval . (load-file (concat c4stl-dir ".project.el")))
         ;(eval . (message "c4stl root: %s" c4stl-dir))
         ;(eval . (message "company-clang-arguments: %s" company-clang-arguments))
         )
      ))

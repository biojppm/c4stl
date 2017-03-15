(setq c4stl-dir (file-name-directory (buffer-file-name)))
(message "c4stl root: %s" c4stl-dir)

(setq company-clang-arguments
      (list
       "-std=c++11"
       (concat "-I" c4stl-dir "src/")
       (concat "-I" c4stl-dir "extern/")
       (concat "-I" c4stl-dir "test/")
       )
      )
(message "company-clang-arguments: %s" company-clang-arguments)

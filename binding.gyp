{
  "targets": [
    {
      "target_name": "utun",
      "sources": [
        "utun.cc"
      ],
      "include_dirs": ["<!(node -e \"require('nan')\")"]
    }
  ]
}
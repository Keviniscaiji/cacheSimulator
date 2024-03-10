const express = require('express');
const morgan = require('morgan');
const mongoose = require('mongoose');
const { exec } = require('child_process');
const app = express();
const port = 3000;
app.set('view engine', 'ejs');
app.get('/', (req, res) => {
    res.render('index');
})

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});

const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");
const HALF_TLOS = ethers.utils.parseEther("0.5");


describe("NativeOracleBridge Contract", function () {
    let bridge, owner, oracle, oracle2, user, testerContract;
    beforeEach(async () => {
        [owner, oracle, oracle2, user] = await ethers.getSigners();
        let TesterContract = await ethers.getContractFactory("NativeOracleBridgeTester");
        let Bridge = await ethers.getContractFactory("NativeOracleBridge");
        bridge = await Bridge.deploy(ONE_TLOS);
        testerContract = await TesterContract.deploy(bridge.address);
    })
    describe(":: Settings", function () {
        it("Owner should be able to register an Oracle" , async function () {
            await expect(bridge.registerOracle(oracle.address, "mygreatoracle")).to.not.be.reverted;
        });
        it("No other address should be able to register an Oracle" , async function () {
            await expect(bridge.connect(user).registerOracle(oracle2.address, "mygreatoracle2")).to.be.reverted;
        });
        it("No other address should be able to remove an Oracle" , async function () {
            await expect(bridge.connect(user).removeOracle(oracle.address)).to.be.reverted;
        });
        it("Owner should be able to remove an Oracle" , async function () {
            await expect(bridge.removeOracle(oracle.address)).to.not.be.reverted;
        });
        it("You should be able to query the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
    });
    describe(":: Getters", function () {
        it("You should be able to query the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
    });
    describe(":: Request", function () {
        it("Request should be created with correct parameters" , async function () {
            await expect(bridge.registerOracle(oracle.address, "mygreatoracle")).to.not.be.reverted;
            await expect(testerContract.makeRequest("helloworld", oracle.address, ["arg1", "arg2"], {"value": ONE_TLOS})).to.not.be.reverted;
        });
        it("Request should revert if fee is incorrect" , async function () {
            await expect(testerContract.makeRequest("helloworld2", oracle.address, ["arg1", "arg2"], {"value": HALF_TLOS})).to.be.reverted;
        });
        it("Request should revert if oracle is not registered" , async function () {
            await expect(testerContract.makeRequest("helloworld2", oracle2.address, ["arg1", "arg2"], {"value": ONE_TLOS})).to.be.reverted;
        });
        it("Request should revert if callId already exists" , async function () {
            await expect(testerContract.makeRequest("helloworld", oracle.address, ["arg1", "arg2"], {"value": ONE_TLOS})).to.be.reverted;
        });
    });
    describe(":: Response", function () {
        it("Response should delete the Request" , async function () {
        });
    });
});
